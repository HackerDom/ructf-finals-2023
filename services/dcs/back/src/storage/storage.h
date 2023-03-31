#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <random>

class Storage {
public:
    explicit Storage(std::filesystem::path storagePath);

    struct SaveResult {
        std::string Error;
        std::string Token;
    };
    std::shared_ptr<SaveResult> SaveNotExecuted(const std::vector<uint8_t> &code, std::string_view description);

//    struct UpdateResult {
//        std::string Error;
//    };
//    std::shared_ptr<UpdateResult> UpdateValue(std::string_view token, double value);

    struct GetResult {
        std::string Error;
        std::vector<uint8_t> Code;
        std::string Description;
        bool IsExecuted;
        double Value;
    };
    std::shared_ptr<GetResult> Get(std::string_view token);

    bool Remove(std::string_view token, std::string &err);
private:
    const std::filesystem::path storagePath;
    std::random_device randomDevice;
    std::mt19937 randomGenerator;
    std::uniform_int_distribution<std::mt19937::result_type> distributionOfRandomGenerator;

    static constexpr int kTokenLength = 36;
    static constexpr int kTokenGenerateMaxAttempts = 3;
    static constexpr std::string_view kTokenAlphabet = "qwertyuiopasdfghjklzxcvbnm1234567890";
    std::string GenerateToken();
    int GenerateUniqueToken(std::string &out);
    std::filesystem::path GetTokenByPath(std::string_view token);
};
