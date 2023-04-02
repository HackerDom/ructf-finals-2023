#include <iostream>

#include <glog/logging.h>
#include <httplib.h>
#include <thirdparty/nlohmann/json.hpp>

#include <options/options.h>
#include <config/config.h>


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

    httplib::Server svr;
    svr.new_task_queue = [config]() { return new httplib::ThreadPool(config->WorkersCount); };

    svr.set_logger([](const httplib::Request &req, const httplib::Response &res) {
        LOG(INFO) << req.path << " | " << req.body;
        LOG(INFO) << res.status << " | " << res.body;
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

    svr.listen(config->ServerAddress, config->ServerPort);

    return 0;
}
