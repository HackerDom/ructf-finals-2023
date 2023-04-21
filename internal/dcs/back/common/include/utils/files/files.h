#pragma once

#include <string>
#include <filesystem>

bool WriteAllToFile(const std::filesystem::path &path, const std::string &content);
std::filesystem::path GetTempUniquePath(const std::string &suffix);
