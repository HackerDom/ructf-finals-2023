#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <memory>

struct TranslationResult {
    bool success;
    std::string errorMessage;
    std::shared_ptr<std::vector<uint8_t>> translated;
};
TranslationResult TranslateAssembly(const std::string &assembly);
