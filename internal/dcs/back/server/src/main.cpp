#include <iostream>
#include <csignal>

#include <glog/logging.h>
#include <httplib.h>
#include <thirdparty/nlohmann/json.hpp>

#include <lexer/lexer.h>
#include <parser/parser.h>
#include <compiler/compiler.h>
#include <translator/translator.h>
#include <config/config.h>
#include <options/options.h>
#include <storage/storage.h>
#include <playground/executor.h>
#include <utils/strings/strings.h>

static const std::string kTokenHeader = "X-BCS-Token";

httplib::Server svr;

void sigintHandler(int) {
    LOG(INFO) << "Closing server...";
    svr.stop();
}

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    FLAGS_logtostdout = true;
    FLAGS_minloglevel = 0;

    auto options = ReadOptionsFromArgs(argc, argv);
    if (options == nullptr) {
        return 1;
    }

    LOG(INFO) << "Config path: " << options->ConfigPath;

    std::ifstream configFile(options->ConfigPath);
    nlohmann::json configJson = nlohmann::json::parse(configFile);
    LOG(INFO) << "Using config: " << configJson;
    configFile.close();

    auto config = std::make_shared<Config>(configJson);
    auto executor = std::make_shared<Executor>(config->WorkersCount);
    auto storage = std::make_shared<Storage>(config->StoragePath);

    svr.new_task_queue = [config]() { return new httplib::ThreadPool(config->WorkersCount); };

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

    svr.Post("/api/compiler", [storage](const httplib::Request &request, httplib::Response &response) {
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
        auto s = storage->SaveNotExecuted(*translated.Translated, j["description"].get<std::string_view>());
        if (s->Status == Storage::OperationStatus::Error) {
            LOG(ERROR) << s->Error;
            response.status = 500;
            return;
        }
        if (s->Status == Storage::OperationStatus::NeedRetry) {
            LOG(WARNING) << "need retry at record creation " << s->Error;
            response.status = 503;
            return;
        }

        response.status = 201;
        response.set_header(kTokenHeader, s->Token);
        return;
    });

    svr.Post("/api/playground", [executor, storage](const httplib::Request &request, httplib::Response &response) {
        auto token = request.get_header_value(kTokenHeader);
        if (token.empty()) {
            response.status = 400;
            response.set_content("expected token", "text/plain");
            return;
        }
        auto record = storage->Get(token);
        if (record->Status == Storage::OperationStatus::Error) {
            LOG(ERROR) << record->Error;
            response.status = 500;
            return;
        }
        if (record->Status == Storage::OperationStatus::NeedRetry) {
            LOG(WARNING) << "collision by token " << token;
            response.status = 503;
            return;
        }
        auto execute = executor->Execute(record->Code);
        if (execute->Status == Executor::ExecuteResult::NO_FREE_EXECUTORS) {
            LOG(WARNING) << "no free executors left";
            response.status = 503;
            return;
        }
        if (execute->Status == Executor::ExecuteResult::CRASH || execute->Status == Executor::ExecuteResult::TIMEOUT) {
            LOG(WARNING) << execute->ErrorMessage;
            response.status = 200;
            response.set_content("program timeout or crash", "text/plain");
            return;
        }
        if (execute->Status == Executor::ExecuteResult::START_ERROR) {
            LOG(ERROR) << execute->ErrorMessage;
            response.status = 500;
            return;
        }
        std::string err;
        if (!storage->UpdateValue(token, execute->Value, err)) {
            LOG(ERROR) << "cant save result: " << err;
            response.status = 500;
            return;
        }
    });

    svr.Get("/api/playground", [storage](const httplib::Request &request, httplib::Response &response) {
        auto token = request.get_header_value(kTokenHeader);
        if (token.empty()) {
            response.status = 400;
            response.set_content("expected token", "text/plain");
            return;
        }
        auto result = storage->Get(token);
        if (result->Status == Storage::OperationStatus::NeedRetry) {
            LOG(WARNING) << "collision by token " << token;
            response.status = 503;
            return;
        }
        if (result->Status == Storage::OperationStatus::Error) {
            LOG(ERROR) << result->Error;
            response.status = 500;
            return;
        }

        nlohmann::json j{
                {"is_executed", result->IsExecuted}
        };
        if (result->IsExecuted) {
            std::ostringstream ss;
            ss << std::setprecision(30) << result->Value;
            j["value"] = ss.str();
        }

        response.status = 200;
        response.set_content(j.dump(), "application/json");
        return;
    });

    LOG(INFO) << "Starting listening, use CTRL-C to close...";

    std::signal(SIGINT, sigintHandler);

    svr.listen(config->ServerAddress, config->ServerPort);

    return 0;
}
