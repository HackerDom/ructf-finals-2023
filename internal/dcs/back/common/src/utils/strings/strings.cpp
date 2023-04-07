#include <cstring>

#include <utils/strings/strings.h>

std::string GetErrnoDescription() {
    char buff[256];
    return std::string{strerror_r(errno, buff, sizeof(buff))};
}

std::string PError(const char *prefix) {
    char buff[256];
    return Format("%s: %s", prefix, strerror_r(errno, buff, sizeof(buff)));
}

std::string PError(int err, const char *prefix) {
    char buff[256];
    return Format("%s: %s", prefix, strerror_r(err, buff, sizeof(buff)));
}