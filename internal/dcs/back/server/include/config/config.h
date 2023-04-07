#pragma once

#include <filesystem>

#include <thirdparty/nlohmann/json.hpp>

struct Config {
    explicit Config(const nlohmann::json &json);

    std::string ServerAddress;
    int ServerPort;
    int WorkersCount;
    std::filesystem::path StoragePath;
};
