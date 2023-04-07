#include <thread>

#include <config/config.h>

Config::Config(const nlohmann::json &json) {
    if (json.count("address") && json["address"].is_string()) {
        ServerAddress = json["address"].get<std::string>();
    } else {
        throw std::runtime_error("no key 'address' in json or it has invalid type");
    }

    if (json.count("port") && json["port"].is_number_integer()) {
        ServerPort = json["port"].get<int>();
    } else {
        throw std::runtime_error("no key 'port' in json or it has invalid type");
    }

    if (json.count("workers_count") && json["workers_count"].is_number_integer()) {
        WorkersCount = json["workers_count"].get<int>();
    } else {
        WorkersCount = static_cast<int>(std::thread::hardware_concurrency());
    }

    if (json.count("storage_path") && json["storage_path"].is_string()) {
        StoragePath = json["storage_path"].get<std::string>();
    } else {
        throw std::runtime_error("no key 'storage_path' in json or it has invalid type");
    }
}