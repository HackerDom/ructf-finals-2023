#pragma once

#include <chrono>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <stdexcept>

class ExecutorError : public std::runtime_error {
public:
    explicit ExecutorError(const std::string &msg) : std::runtime_error(msg) {
    }
};

class Executor final {
public:
    explicit Executor(std::size_t executorsAmount);

    ~Executor();

    struct ExecuteResult {
        enum StatusCode {
            OK,
            NO_FREE_EXECUTORS,
            START_ERROR,
            TIMEOUT,
            CRASH
        };

        double Value;
        std::string ErrorMessage;
        StatusCode Status;
    };

    [[nodiscard]] std::shared_ptr<ExecuteResult> Execute(const std::vector<uint8_t> &code);

private:
    static constexpr std::size_t kPagesPerExecutor = 100;
    static_assert(kPagesPerExecutor > 1);

    struct {
        void *ptr{nullptr};
        std::size_t size;
        std::mutex mutex;
    } arena{};

    struct ExecutionUnit {
        bool free;
        void *executable;
        std::size_t executableSize;
        void *writable;

        [[nodiscard]] std::shared_ptr<ExecuteResult> Execute(const std::vector<uint8_t> &code);
    };

    struct {
        std::vector<ExecutionUnit> u;
        std::mutex mutex;
    } units{};

    std::atomic<int> runningInstances;
    std::atomic<bool> acceptNewInstances;

    const std::size_t pageSize;
};
