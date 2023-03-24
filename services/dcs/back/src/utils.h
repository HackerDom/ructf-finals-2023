#pragma once

#include <memory>
#include <string>
#include <stdexcept>

template<typename ... Args>
std::string Format(const std::string &format, Args ... args) {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
    if (size_s <= 0) {
        throw std::runtime_error("Format string error");
    }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);

    return {buf.get(), buf.get() + size - 1};
}

static inline void LTrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

static inline void RTrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static inline void Trim(std::string &s) {
    RTrim(s);
    LTrim(s);
}

static inline std::string LTrimCopy(std::string s) {
    LTrim(s);
    return s;
}

static inline std::string RTrimCopy(std::string s) {
    RTrim(s);
    return s;
}

static inline std::string TrimCopy(std::string s) {
    Trim(s);
    return s;
}