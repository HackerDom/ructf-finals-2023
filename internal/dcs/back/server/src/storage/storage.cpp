#include <string_view>
#include <cstring>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>

#include <utils/defer/defer.h>
#include <utils/strings/strings.h>

#include <storage/storage.h>


std::filesystem::path Storage::GetPathByToken(std::string_view token) {
    return storagePath / token.substr(0, 2) / token.substr(2);
}

std::string Storage::GenerateToken() {
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, kTokenAlphabet.size() - 1);
    std::string token;

    for (int i = 0; i < kTokenLength; ++i) {
        token += kTokenAlphabet[dist(randomGenerator)];
    }

    return token;
}

int Storage::GenerateUniqueToken(std::string &out) {
    for (int attempt = 0; attempt < kTokenGenerateMaxAttempts; ++attempt) {
        auto token = GenerateToken();
        auto path = GetPathByToken(token);

        if (std::filesystem::is_regular_file(path)) {
            continue;
        }

        out = token;
        return 0;
    }

    return 1;
}

Storage::Storage(std::filesystem::path storagePath) : storagePath(std::move(storagePath)), randomDevice(),
                                                      randomGenerator(randomDevice()) {
    if (std::error_code errorCode;
        !std::filesystem::is_directory(this->storagePath) &&
        !std::filesystem::create_directory(this->storagePath, errorCode)) {
        throw std::runtime_error(errorCode.message());
    }
}

std::shared_ptr<Storage::SaveResult>
Storage::Save(const std::vector<uint8_t> &code, std::string_view description) {
    std::string token;
    if (GenerateUniqueToken(token) != 0) {
        return std::make_shared<SaveResult>(SaveResult{Error, "cant generate token"});
    }
    auto pathToSave = GetPathByToken(token);

    std::error_code dirCreateErr;
    if (!std::filesystem::is_directory(pathToSave.parent_path()) &&
        !std::filesystem::create_directory(pathToSave.parent_path(), dirCreateErr)) {
        return std::make_shared<SaveResult>(
                SaveResult{Error, Format("cant create directory: %s", dirCreateErr.message().c_str())});
    }

    int fd = open(pathToSave.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        return std::make_shared<SaveResult>(SaveResult{Error, PError("open")});
    }
    Defer fileClose(close, fd);

    int lockSt = flock(fd, LOCK_EX | LOCK_NB);
    if (lockSt == -1) {
        if (errno == EWOULDBLOCK) {
            return std::make_shared<SaveResult>(SaveResult{Error, "flock EWOULDBLOCK: possible token race"});
        }
        return std::make_shared<SaveResult>(SaveResult{Error, PError("flock")});
    }
    Defer unlock(flock, fd, LOCK_UN);

    // std::string can contains zeros
    // Get reads only to first zero byte, other bytes will be treated as code
    // to prevent code injection, we must cut description to first zero
    auto descriptionLen = std::strlen(description.data());
    auto totalLength = descriptionLen + 1 + code.size();
    int allocErr = posix_fallocate(fd, 0, static_cast<off_t>(totalLength));
    if (allocErr != 0) {
        return std::make_shared<SaveResult>(SaveResult{Error, PError(allocErr, "posix_fallocate")});
    }

    auto mapped = mmap(nullptr, totalLength, PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        return std::make_shared<SaveResult>(SaveResult{Error, PError("mmap")});
    }
    Defer um(munmap, mapped, totalLength);

    auto mappedBytePtr = reinterpret_cast<uint8_t *>(mapped);
    std::memcpy(mappedBytePtr, description.data(), descriptionLen);
    mappedBytePtr[descriptionLen] = 0;
    std::memcpy(mappedBytePtr + descriptionLen + 1, code.data(), code.size());

    return std::make_shared<SaveResult>(SaveResult{Success, "", std::move(token)});
}

std::shared_ptr<Storage::GetResult> Storage::Get(std::string_view token) {
    if (token.size() != kTokenLength) {
        return std::make_shared<GetResult>(GetResult{InvalidToken, "invalid token"});
    }
    auto path = GetPathByToken(token);
    if (!std::filesystem::is_regular_file(path)) {
        return std::make_shared<GetResult>(GetResult{InvalidToken, "invalid token"});
    }

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        return std::make_shared<GetResult>(GetResult{Error, PError("open")});
    }
    Defer fileClose(close, fd);

    struct stat statResult{};
    if (fstat(fd, &statResult) == -1) {
        return std::make_shared<GetResult>(GetResult{Error, PError("fstat")});
    }

    auto mapped = mmap(nullptr, statResult.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        return std::make_shared<GetResult>(GetResult{Error, PError("mmap")});
    }
    Defer um(munmap, mapped, statResult.st_size);

    auto mappedBytePtr = reinterpret_cast<uint8_t *>(mapped);
    auto result = std::make_shared<GetResult>(GetResult{});
    auto descriptionLen = std::strlen(reinterpret_cast<const char*>(mappedBytePtr));
    result->Description.resize(descriptionLen);
    result->Code.resize(statResult.st_size - descriptionLen - 1);
    std::memcpy(result->Description.data(), mappedBytePtr, result->Description.size());
    std::memcpy(result->Code.data(), mappedBytePtr + descriptionLen + 1, result->Code.size());

    return result;
}
