#include <fstream>
#include <random>

#include <utils/files/files.h>

namespace fs = std::filesystem;

bool WriteAllToFile(const fs::path &path, const std::string &content) {
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file) {
        return false;
    }
    file << content;
    file.close();
    return true;
}

static std::string randomString() {
    static const std::string_view alpha = "qwertyuiopasdfghjklzxcvbnm";
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, alpha.size() - 1);
    std::string result;

    result.reserve(8);

    for (std::size_t i = 0; i < result.capacity(); ++i) {
        result += alpha[dist(rng)];
    }

    return result;
}

fs::path GetTempUniquePath(const std::string &suffix) {
    auto count = 0;

    while (count < 3) {
        auto name = randomString() + suffix;
        auto path = fs::temp_directory_path() / name;

        if (!fs::is_regular_file(path) && !fs::is_directory(path)) {
            return path;
        }

        ++count;
    }

    return {};
}