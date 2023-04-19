#include <iostream>
#include <csignal>

#include <glog/logging.h>
#include <httplib.h>
#include <thirdparty/nlohmann/json.hpp>

#include <lexer/lexer.h>
#include <parser/parser.h>
#include <compiler/compiler.h>
#include <translator/translator.h>
#include <options/options.h>
#include <storage/storage.h>
#include <playground/executor.h>
#include <utils/strings/strings.h>

static constexpr std::string_view kStoragePath = "/var/dcs/data";

static const std::string kTokenHeader = "X-BCS-Token";

httplib::Server svr;

static void sigintHandler(int) {
    LOG(INFO) << "Closing server...";
    svr.stop();
}

static std::string doubleToString(double v) {
    std::ostringstream ss;
    ss << std::setprecision(30) << v;
    return ss.str();
}

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    FLAGS_logtostderr = true;
    FLAGS_minloglevel = 0;

    auto options = ReadOptionsFromArgs(argc, argv);
        if (options == nullptr) {
        return 1;
    }

    LOG(INFO) << "workers count: " << options->WorkersCount;

    auto executor = std::make_shared<Executor>(options->WorkersCount);
    auto storage = std::make_shared<Storage>(kStoragePath);

    svr.new_task_queue = [options]() { return new httplib::ThreadPool(options->WorkersCount); };

    svr.set_logger([](const httplib::Request &req, const httplib::Response &res) {
        auto printHeaders = [](const httplib::Headers &h) {
            std::ostringstream ss;

            for (const auto &it : h) {
                ss << "(" << it.first << ": " << it.second << ")";
            }

            return ss.str();
        };
        LOG(INFO) << Format("==> %s %s headers{%s} body{%s}", req.method.c_str(), req.path.c_str(), printHeaders(req.headers).c_str(), req.body.c_str());
        LOG(INFO) << Format("<== %d headers{%s} body{%s}", res.status, printHeaders(res.headers).c_str(), res.body.c_str());
    });

    svr.Post("/api/compute", [storage](const httplib::Request &request, httplib::Response &response) {
        if (request.get_header_value("Content-Type") != "application/json") {
            response.status = 400;
            response.set_content("invalid content type", "text/plain");
            return;
        }
        auto j = nlohmann::json::parse(request.body);
        if (!j.contains("description") || !j["description"].is_string()) {
            response.status = 400;
            response.set_content("no description field", "text/plain");
            return;
        }
        if (!j.contains("code") || !j["code"].is_string()) {
            response.status = 400;
            response.set_content("no code field", "text/plain");
            return;
        }
        auto description = j["description"].get<std::string_view>();
        if (description.size() > 10'000) {
            response.status = 400;
            response.set_content("description too large", "text/plain");
            return;
        }
        auto text = j["code"].get<std::string_view>();
        if (text.size() > 10'000) {
            response.status = 400;
            response.set_content("code too large", "text/plain");
            return;
        }
        auto tokens = TokenizeString(text);
        if (!tokens.Success) {
            response.status = 400;
            response.set_content(tokens.ErrorMessage, "text/plain");
            return;
        }
        auto parsed = ParseTokens(tokens.Tokens);
        if (!parsed.Success) {
            response.status = 400;
            response.set_content(parsed.ErrorMessage, "text/plain");
            return;
        }
        auto compiled = CompileToAssembly(parsed.ProgramNode);
        if (!compiled.Success) {
            response.status = 400;
            response.set_content(compiled.ErrorMessage, "text/plain");
            return;
        }
        auto translated = TranslateAssembly(compiled.AssemblyCode);
        if (!translated.Success) {
            response.status = 500;
            LOG(ERROR) << "translation failed: " << translated.ErrorMessage;
            return;
        }
        auto s = storage->Save(*translated.Translated, description);
        if (s->Status == Storage::OperationStatus::Error) {
            LOG(ERROR) << s->Error;
            response.status = 500;
            return;
        }

        response.status = 201;
        response.set_header(kTokenHeader, s->Token);
        return;
    });

    svr.Get("/api/compute", [executor, storage](const httplib::Request &request, httplib::Response &response) {
        auto token = request.get_header_value(kTokenHeader);
        if (token.empty()) {
            response.status = 400;
            nlohmann::json j{
                {"status", "error"},
                {"message", "token expected"}
            };
            response.set_content(j.dump(), "application/json");
            return;
        }
        auto record = storage->Get(token);
        if (record->Status == Storage::OperationStatus::InvalidToken) {
            response.status = 400;
            nlohmann::json j{
                {"status", "error"},
                {"message", "invalid token"}
            };
            response.set_content(j.dump(), "application/json");
            return;
        }
        if (record->Status == Storage::OperationStatus::Error) {
            LOG(ERROR) << record->Error;
            response.status = 500;
            nlohmann::json j{
                {"status", "error"},
                {"message", "internal error"}
            };
            response.set_content(j.dump(), "application/json");
            return;
        }
        auto execute = executor->Execute(record->Code);
        if (execute->Status == Executor::ExecuteResult::NO_FREE_EXECUTORS) {
            LOG(ERROR) << "no free executors left";
            response.status = 503;
            nlohmann::json j{
                {"status", "error"},
                {"message", "executors busy"}
            };
            response.set_content(j.dump(), "application/json");
            return;
        }
        if (execute->Status == Executor::ExecuteResult::TIMEOUT || execute->Status == Executor::ExecuteResult::CRASH) {
            LOG(WARNING) << execute->ErrorMessage;
            response.status = 200;
            nlohmann::json j{
                {"status", "error"},
                {"description", record->Description},
                {"message", "program timeout or crash"}
            };
            response.set_content(j.dump(), "application/json");
            return;
        }
        if (execute->Status == Executor::ExecuteResult::START_ERROR) {
            LOG(ERROR) << execute->ErrorMessage;
            response.status = 500;
            nlohmann::json j{
                {"status", "error"},
                {"message", "internal error"}
            };
            response.set_content(j.dump(), "application/json");
            return;
        }

        nlohmann::json j{
                {"status", "success"},
                {"description", record->Description},
                {"computed_value", doubleToString(execute->Value)}
        };

        response.status = 200;
        response.set_content(j.dump(), "application/json");
    });

    svr.set_error_handler([](const auto& req, auto& res) {});

    svr.set_exception_handler([](const auto& req, auto& res, std::exception_ptr ep) {
        try {
            std::rethrow_exception(ep);
        } catch (std::exception &e) {
            LOG(ERROR) << "unhandled std::exception in " << req.path << ": " << e.what();
        } catch (...) {
            LOG(ERROR) << "unhandled exception in " << req.path;
        }
        res.set_content("internal server error", "text/html");
        res.status = 500;
    });

    LOG(INFO) << "Starting listening, use CTRL-C to close...";

    std::signal(SIGINT, sigintHandler);
    std::signal(SIGTERM, sigintHandler);

    svr.listen("0.0.0.0", 7654);

    return 0;
}
