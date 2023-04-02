#include <unistd.h>

#include <glog/logging.h>

#include <utils/strings/strings.h>

#include <options/options.h>

std::shared_ptr<ServerOptions> ReadOptionsFromArgs(int argc, char **argv) {
    std::string configPath;

    int opt;
    while ((opt = getopt(argc, argv, "hc:")) != -1) {
        switch (opt) {
            case 'c':
                configPath = std::string{optarg};
                break;
            case 'h':
                LOG(INFO) << Format("usage: %s -c <config file path>", argv[0]);
                return nullptr;
            case '?':
                LOG(ERROR) << "error parsing arguments";
                return nullptr;
            default:
                LOG(ERROR) << "unknown argument: " << opt;
                return nullptr;
        }
    }

    if (configPath.empty()) {
        LOG(ERROR) << "use -c to specify config path";
        return nullptr;
    }

    auto options = std::make_shared<ServerOptions>();

    options->ConfigPath = configPath;

    return options;
}