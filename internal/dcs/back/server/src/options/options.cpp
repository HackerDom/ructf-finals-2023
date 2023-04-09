#include <unistd.h>

#include <glog/logging.h>

#include <utils/strings/strings.h>

#include <options/options.h>

std::shared_ptr<ServerOptions> ReadOptionsFromArgs(int argc, char **argv) {
    std::string workersCount;

    int opt;
    while ((opt = getopt(argc, argv, "hw:")) != -1) {
        switch (opt) {
            case 'w':
                workersCount = std::string{optarg};
                break;
            case 'h':
                LOG(INFO) << Format("usage: %s -w <workers count>", argv[0]);
                return nullptr;
            case '?':
                LOG(ERROR) << "error parsing arguments";
                return nullptr;
            default:
                LOG(ERROR) << "unknown argument: " << opt;
                return nullptr;
        }
    }

    if (workersCount.empty()) {
        LOG(ERROR) << "use -w to specify workers count";
        return nullptr;
    }

    auto options = std::make_shared<ServerOptions>();

    options->WorkersCount = std::stoi(workersCount);

    return options;
}