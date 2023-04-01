#include <cstdio>

#include <utils/process/process.h>

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