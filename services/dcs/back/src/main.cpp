#include <iostream>

#include <glog/logging.h>

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    FLAGS_logtostdout = true;
    FLAGS_minloglevel = 0;

    LOG(ERROR) << "some error!";
    return 0;
}
