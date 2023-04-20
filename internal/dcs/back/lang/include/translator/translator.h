#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <memory>

struct TranslationResult {
    bool Success;
    std::string ErrorMessage;
    std::shared_ptr<std::vector<uint8_t>> Translated;
};
TranslationResult TranslateAssembly(const std::string &assembly);
