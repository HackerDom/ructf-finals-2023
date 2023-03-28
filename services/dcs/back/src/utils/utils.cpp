#include <random>
#include <fstream>
#include <cstdio>

#include <dlfcn.h>

#include "utils.h"

static std::string randomString() {
    static const std::string alpha = "qwertyuiopasdfghjklzxcvbnm";
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

std::filesystem::path GetTempUniquePath(const std::string &suffix) {
    auto count = 0;

    while (count < 3) {
        auto name = randomString() + suffix;
        auto path = std::filesystem::temp_directory_path() / name;

        if (!std::filesystem::is_regular_file(path)) {
            return path;
        }

        ++count;
    }

    return {};
}

std::string GetErrnoDescription() {
    char buff[256];
    return std::string{strerror_r(errno, buff, sizeof(buff))};
}

bool WriteAllToFile(const std::filesystem::path &path, const std::string &content) {
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file) {
        return false;
    }
    file << content;
    file.close();
    return true;
}

void ExecuteAndGetStdout(const std::string &cmd, std::string &out) {
    std::array<char, 128> buffer{};
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        out += std::string(buffer.data());
    }
}

DynamicLibrary::DynamicLibrary(std::filesystem::path path) : Path(std::move(path)) {
    libPtr = dlopen(Path.c_str(), RTLD_NOW);

    if (libPtr == nullptr) {
        error = Format("cant open shared library %s: %s", Path.c_str(), dlerror());
    }
}

DynamicLibrary::~DynamicLibrary() {
    if (libPtr != nullptr) {
        dlclose(libPtr);
    }
}

DynamicLibrary::SymbolLoadResult DynamicLibrary::LoadSymbol(const char *name) const {
    void *sym = dlsym(libPtr, name);

    if (sym != nullptr) {
        return {sym};
    }

    return {nullptr, std::string(dlerror())};
}
