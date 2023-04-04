#include <iostream>
#include <csignal>

#include <glog/logging.h>
#include <httplib.h>
#include <thirdparty/nlohmann/json.hpp>

#include <options/options.h>
#include <utils/strings/strings.h>
#include <config/config.h>

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

    svr.Post("/api/compiler", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });
    svr.Post("/api/playground", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });
    svr.Get("/api/playground", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });

    LOG(INFO) << "Starting listening, use CTRL-C to close...";

    std::signal(SIGINT, sigintHandler);

    svr.listen(config->ServerAddress, config->ServerPort);

    return 0;
}
