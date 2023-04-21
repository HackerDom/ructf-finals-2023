#pragma once

#include <string>
#include <stdexcept>
#include <memory>

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

template<class It>
std::string Join(It begin, It end, std::string_view delim) {
    std::string result;

    if (begin == end) {
        return result;
    }

    result += *begin;
    ++begin;
    while (begin != end) {
        result += delim;
        result += *begin;
        ++begin;
    }

    return result;
}

template<class It, class F>
std::string Join(It begin, It end, F func, std::string_view delim) {
    std::string result;

    if (begin == end) {
        return result;
    }

    result += func(*begin);
    ++begin;
    while (begin != end) {
        result += delim;
        result += func(*begin);
        ++begin;
    }

    return result;
}

std::string GetErrnoDescription();

std::string PError(const char *prefix);

std::string PError(int err, const char *prefix);

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
