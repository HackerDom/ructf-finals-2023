#pragma once

#include <memory>

struct ServerOptions {
    int WorkersCount;
};

std::shared_ptr<ServerOptions> ReadOptionsFromArgs(int argc, char **argv);
