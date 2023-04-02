#pragma once

#include <memory>
#include <filesystem>

struct ServerOptions {
    std::filesystem::path ConfigPath;
};

std::shared_ptr<ServerOptions> ReadOptionsFromArgs(int argc, char **argv);
