#include <cstdint>
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

struct RecordHeader {
    static constexpr int kMagicLength = 16;
    static constexpr std::string_view kMagic = "\x07" "DCSPCKT";
    static constexpr int kVersionOffset = kMagic.size();
    static constexpr uint8_t kVersionNum = 1;

    static_assert(kMagic.size() <= kMagicLength);

    uint8_t Magic[kMagicLength];
    uint32_t CodeLength;
    uint32_t CodeOffset;
    uint32_t DescriptionLength;
    uint32_t DescriptionOffset;
    double Value;
    bool IsExecuted;
};

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
                                                      randomGenerator(randomDevice()) {}

void InitializeHeader(const std::vector<uint8_t> &code, const std::string_view &description, RecordHeader &header) {
    memset(&header, 0, sizeof(RecordHeader));
    memcpy(header.Magic, RecordHeader::kMagic.data(), RecordHeader::kMagic.size());
    header.Magic[RecordHeader::kVersionOffset] = RecordHeader::kVersionNum;
    header.IsExecuted = false;
    header.CodeLength = static_cast<uint32_t>(code.size());
    header.DescriptionLength = static_cast<uint32_t>(description.size());
    header.CodeOffset = sizeof(RecordHeader);
    header.DescriptionOffset = header.CodeOffset + header.CodeLength;
}

std::shared_ptr<Storage::SaveResult>
Storage::SaveNotExecuted(const std::vector<uint8_t> &code, std::string_view description) {
    if (code.size() > std::numeric_limits<uint32_t>::max()) {
        return std::make_shared<SaveResult>(SaveResult{Error, "code size is too large"});
    }
    if (description.size() > std::numeric_limits<uint32_t>::max()) {
        return std::make_shared<SaveResult>(SaveResult{Error, "description size is too large"});
    }

    std::string token;
    if (GenerateUniqueToken(token) != 0) {
        return std::make_shared<SaveResult>(SaveResult{NeedRetry, "cant generate token"});
    }
    auto pathToSave = GetPathByToken(token);

    std::error_code dirCreateErr;
    if (!std::filesystem::is_directory(pathToSave.parent_path()) &&
        !std::filesystem::create_directory(pathToSave.parent_path(), dirCreateErr)) {
        return std::make_shared<SaveResult>(
                SaveResult{Error, Format("cant create directory: %s", dirCreateErr.message().c_str())});
    }

    RecordHeader header{};
    InitializeHeader(code, description, header);

    int fd = open(pathToSave.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        return std::make_shared<SaveResult>(SaveResult{Error, PError("open")});
    }
    Defer fileClose(close, fd);

    int lockSt = flock(fd, LOCK_EX | LOCK_NB);
    if (lockSt == -1) {
        if (errno == EWOULDBLOCK) {
            return std::make_shared<SaveResult>(SaveResult{NeedRetry});
        }
        return std::make_shared<SaveResult>(SaveResult{Error, PError("flock")});
    }
    Defer unlock(flock, fd, LOCK_UN);

    auto totalLength = sizeof(RecordHeader) + code.size() + description.size();
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

    std::memcpy(mapped, &header, sizeof(header));
    std::memcpy(mappedBytePtr + header.CodeOffset, code.data(), code.size());
    std::memcpy(mappedBytePtr + header.DescriptionOffset, description.data(), description.size());

    return std::make_shared<SaveResult>(SaveResult{Success, "", std::move(token)});
}

std::shared_ptr<Storage::GetResult> Storage::Get(std::string_view token) {
    auto path = GetPathByToken(token);
    if (!std::filesystem::is_regular_file(path)) {
        return std::make_shared<GetResult>(GetResult{Error, "invalid token"});
    }

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        return std::make_shared<GetResult>(GetResult{Error, PError("open")});
    }
    Defer fileClose(close, fd);

    int lockSt = flock(fd, LOCK_EX | LOCK_NB);
    if (lockSt == -1) {
        if (errno == EWOULDBLOCK) {
            return std::make_shared<GetResult>(GetResult{NeedRetry});
        }
        return std::make_shared<GetResult>(GetResult{Error, PError("flock")});
    }
    Defer unlock(flock, fd, LOCK_UN);

    struct stat statResult{};
    if (fstat(fd, &statResult) == -1) {
        return std::make_shared<GetResult>(GetResult{Error, PError("fstat")});
    }

    auto mapped = mmap(nullptr, statResult.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        return std::make_shared<GetResult>(GetResult{Error, PError("mmap")});
    }
    Defer um(munmap, mapped, statResult.st_size);

    auto *header = reinterpret_cast<RecordHeader *>(mapped);

    if (statResult.st_size < static_cast<off_t>(sizeof(RecordHeader))) {
        return std::make_shared<GetResult>(GetResult{Error, "file too small"});
    }
    if (memcmp(header->Magic, RecordHeader::kMagic.data(), RecordHeader::kMagic.size()) != 0) {
        return std::make_shared<GetResult>(GetResult{Error, "invalid magic"});
    }
    if (header->Magic[RecordHeader::kVersionOffset] != RecordHeader::kVersionNum) {
        return std::make_shared<GetResult>(GetResult{Error, "invalid version"});
    }
    if ((header->CodeOffset + header->CodeLength) > statResult.st_size) {
        return std::make_shared<GetResult>(GetResult{Error, "invalid code offset and size info"});
    }
    if ((header->DescriptionOffset + header->DescriptionLength) > statResult.st_size) {
        return std::make_shared<GetResult>(GetResult{Error, "invalid description offset and size info"});
    }

    auto mappedBytePtr = reinterpret_cast<uint8_t *>(mapped);

    auto result = std::make_shared<GetResult>(GetResult{});
    result->IsExecuted = header->IsExecuted;
    result->Value = header->Value;
    result->Code.resize(header->CodeLength);
    std::memcpy(result->Code.data(), mappedBytePtr + header->CodeOffset, header->CodeLength);
    result->Description.resize(header->DescriptionLength);
    std::memcpy(result->Description.data(), mappedBytePtr + header->DescriptionOffset, header->DescriptionLength);

    return result;
}

Storage::OperationStatus Storage::Remove(std::string_view token, std::string &err) {
    auto path = GetPathByToken(token);
    if (!std::filesystem::is_regular_file(path)) {
        err = "invalid token";
        return Error;
    }

    std::error_code removeErr;
    if (!std::filesystem::remove(path, removeErr)) {
        err = Format("cant remove: %s", removeErr.message().c_str());
        return Error;
    }

    return Success;
}

Storage::OperationStatus Storage::UpdateValue(std::string_view token, double value, std::string &err) {
    auto path = GetPathByToken(token);
    if (!std::filesystem::is_regular_file(path)) {
        err = "invalid token";
        return OperationStatus::Error;
    }

    int fd = open(path.c_str(), O_RDWR);
    if (fd < 0) {
        err = PError("open");
        return OperationStatus::Error;
    }
    Defer fdClose(close, fd);

    int lockSt = flock(fd, LOCK_EX | LOCK_NB);
    if (lockSt == -1) {
        if (errno == EWOULDBLOCK) {
            return NeedRetry;
        }
        err = PError("flock");
        return Error;
    }
    Defer unlock(flock, fd, LOCK_UN);

    struct stat statResult{};
    if (fstat(fd, &statResult) == -1) {
        err = PError("fstat");
        return OperationStatus::Error;
    }

    auto mapped = mmap(nullptr, statResult.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        err = PError("mmap");
        return OperationStatus::Error;
    }
    Defer unmap(munmap, mapped, statResult.st_size);

    auto *header = reinterpret_cast<RecordHeader *>(mapped);
    if (statResult.st_size < static_cast<off_t>(sizeof(RecordHeader))) {
        err = "file too small";
        return OperationStatus::Error;
    }
    if (memcmp(header->Magic, RecordHeader::kMagic.data(), RecordHeader::kMagic.size()) != 0) {
        err = "invalid magic";
        return OperationStatus::Error;
    }
    if (header->Magic[RecordHeader::kVersionOffset] != RecordHeader::kVersionNum) {
        err = "invalid version";
        return OperationStatus::Error;
    }

    header->IsExecuted = true;
    header->Value = value;

    return OperationStatus::Success;
}
