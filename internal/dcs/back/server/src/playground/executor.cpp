#include <chrono>
#include <cstring>
#include <thread>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <glog/logging.h>

#include <utils/defer/defer.h>
#include <utils/strings/strings.h>
#include <playground/executor.h>

using namespace std::chrono_literals;

Executor::Executor(std::size_t executorsAmount) : runningInstances(0), acceptNewInstances(false),
                                                  pageSize(getpagesize()) {
    std::lock_guard _1(arena.mutex);
    std::lock_guard _2(units.mutex);

    arena.size = pageSize * executorsAmount * kPagesPerExecutor;
    arena.ptr = mmap(nullptr, arena.size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    if (arena.ptr == MAP_FAILED) {
        LOG(ERROR) << PError("mmap");
        throw ExecutorError(PError("mmap"));
    }

    units.u.resize(executorsAmount);

    for (std::size_t i = 0; i < executorsAmount; ++i) {
        auto &u = units.u[i];
        u.free = true;
        u.executable = reinterpret_cast<uint8_t *>(arena.ptr) + pageSize * kPagesPerExecutor * i;
        u.executableSize = (kPagesPerExecutor - 1) * pageSize;
        // last page will be used as storage for computed value
        u.writable = reinterpret_cast<uint8_t *>(u.executable) + u.executableSize;
    }

    acceptNewInstances = true;
}

Executor::~Executor() {
    acceptNewInstances = false;

    std::lock_guard l(arena.mutex);

    while (runningInstances.load() != 0) {
        std::this_thread::yield();
    }

    munmap(arena.ptr, arena.size);
}

std::shared_ptr<Executor::ExecuteResult> Executor::Execute(const std::vector<uint8_t> &code) {
    if (!acceptNewInstances.load()) {
        auto result = std::make_shared<Executor::ExecuteResult>();
        result->Status = ExecuteResult::START_ERROR;
        result->ErrorMessage = "Executor is currently destroying";
        return result;
    }

    runningInstances.fetch_add(1);
    Defer sub([this]() { runningInstances.fetch_sub(1); });

    if (!acceptNewInstances.load()) {
        auto result = std::make_shared<Executor::ExecuteResult>();
        result->Status = ExecuteResult::START_ERROR;
        result->ErrorMessage = "Executor is currently destroying";
        return result;
    }

    std::size_t unitIdx = units.u.size();

    LOG(INFO) << "searching for free executor...";
    {
        std::lock_guard l(units.mutex);
        for (std::size_t i = 0; i < units.u.size(); ++i) {
            if (units.u[i].free) {
                unitIdx = i;
                units.u[i].free = false;
                break;
            }
        }
    }

    if (unitIdx == units.u.size()) {
        LOG(WARNING) << "no free executor found";
        auto result = std::make_shared<Executor::ExecuteResult>();
        result->ErrorMessage = "no free executors left";
        result->Status = ExecuteResult::NO_FREE_EXECUTORS;
        return result;
    }
    LOG(INFO) << "free executor #" << unitIdx << " found";
    Defer freeUnit([this, unitIdx] {
        std::lock_guard l(units.mutex);
        units.u[unitIdx].free = true;
        LOG(INFO) << "executor #" << unitIdx << " freed";
    });

    return units.u[unitIdx].Execute(code);
}

[[nodiscard]] std::shared_ptr<Executor::ExecuteResult> Executor::ExecutionUnit::Execute(
        const std::vector<uint8_t> &code) {
    auto result = std::make_shared<Executor::ExecuteResult>();

    if (code.size() > executableSize) {
        LOG(WARNING) << "too large code for execution";
        result->Status = ExecuteResult::START_ERROR;
        result->ErrorMessage = "code too large";
        return result;
    }

    if (mprotect(executable, executableSize, PROT_WRITE | PROT_READ) != 0) {
        LOG(ERROR) << PError("mprotect(PROT_WRITE | PROT_READ)");
        result->Status = ExecuteResult::START_ERROR;
        result->ErrorMessage = PError("mprotect(PROT_WRITE | PROT_READ)");
        return result;
    }

    std::memset(executable, 0, executableSize);
    std::memcpy(executable, code.data(), code.size());

    if (mprotect(executable, executableSize, PROT_READ | PROT_EXEC) != 0) {
        LOG(ERROR) << PError("mprotect(PROT_READ | PROT_EXEC)");
        result->Status = ExecuteResult::START_ERROR;
        result->ErrorMessage = PError("mprotect(PROT_READ | PROT_EXEC)");
        return result;
    }

    LOG(INFO) << "starting child process...";

    auto pid = fork();
    if (pid == -1) {
        LOG(ERROR) << PError("fork failed");
        result->Status = ExecuteResult::START_ERROR;
        result->ErrorMessage = PError("fork");
        return result;
    }

    if (pid == 0) {
        auto fptr = reinterpret_cast<double(*)()>(executable);
        (*reinterpret_cast<double*>(writable)) = fptr();

        std::exit(0);
    }

    int status = 0;

    auto sleepPeriod = 10ms;
    auto totalWaitTime = 0ms;

    LOG(INFO) << "child process started, pid = " << pid;
    while (totalWaitTime < 100ms) {
        LOG(INFO) << "checking status of " << pid;
        auto st = waitpid(pid, &status, WNOHANG);
        if (st == -1) {
            LOG(ERROR) << "cannot check status, killing process...";
            if (kill(pid, SIGKILL) != 0) {
                LOG(ERROR) << PError("kill(SIGKILL)");
            } else {
                waitpid(pid, nullptr, 0);
                LOG(INFO) << pid << " killed";
            }
            LOG(ERROR) << PError("waitpid");
            result->Status = ExecuteResult::START_ERROR;
            result->ErrorMessage = PError("waitpid");
            return result;
        }
        if (st == pid) {
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                LOG(INFO) << "process " << pid << " finished successfully";
                result->Value = (*reinterpret_cast<double*>(writable));
                result->Status = ExecuteResult::OK;
                return result;
            }
            LOG(ERROR) << "process " << pid << " failed";
            result->Status = ExecuteResult::CRASH;
            result->ErrorMessage = "crash, check for stack overflow";
            return result;
        }

        LOG(INFO) << "wait for " << pid << " finished...";
        std::this_thread::sleep_for(sleepPeriod);
        totalWaitTime += sleepPeriod;
    }

    LOG(INFO) << pid << " has not completed in timeout, killing...";
    if (kill(pid, SIGKILL) != 0) {
        LOG(ERROR) << PError("kill(SIGKILL)");
    }
    // collect status for not create a zombie process
    waitpid(pid, nullptr, 0);

    result->Status = ExecuteResult::TIMEOUT;

    return result;
}