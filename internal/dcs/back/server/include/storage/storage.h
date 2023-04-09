#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <random>

class Storage {
public:
    enum OperationStatus {
        Success,
        InvalidToken,
        Error
    };

    explicit Storage(std::filesystem::path storagePath);

    struct SaveResult {
        OperationStatus Status;
        std::string Error;
        std::string Token;
    };
    std::shared_ptr<SaveResult> Save(const std::vector<uint8_t> &code, std::string_view description);

    struct GetResult {
        OperationStatus Status;
        std::string Error;
        std::vector<uint8_t> Code;
        std::string Description;
    };
    std::shared_ptr<GetResult> Get(std::string_view token);

private:
    const std::filesystem::path storagePath;
    std::random_device randomDevice;
    std::mt19937 randomGenerator;
    std::uniform_int_distribution<std::mt19937::result_type> distributionOfRandomGenerator;

    static constexpr int kTokenLength = 16;
    static constexpr int kTokenGenerateMaxAttempts = 3;
    static constexpr std::string_view kTokenAlphabet = "qwertyuiopasdfghjklzxcvbnm1234567890";
    std::string GenerateToken();
    int GenerateUniqueToken(std::string &out);
    std::filesystem::path GetPathByToken(std::string_view token);
};
