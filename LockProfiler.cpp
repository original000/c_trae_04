#include "LockProfiler.h"
#include "StackWalker.h"
#include <fstream>
#include <algorithm>
#include <sstream>

LockProfiler::LockProfiler() : isRunning_(false) {
}

LockProfiler::~LockProfiler() {
    Stop();
}

void LockProfiler::Start() {
    if (!isRunning_.exchange(true)) {
        samplingThread_ = std::thread(&LockProfiler::SamplingThread, this);
    }
}

bool LockProfiler::IsRunning() const {
    return isRunning_;
}

void LockProfiler::Stop() {
    if (isRunning_.exchange(false)) {
        if (samplingThread_.joinable()) {
            samplingThread_.join();
        }
    }
}

void LockProfiler::OnLockAcquired(void* lockAddr) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    lockOwners_[lockAddr] = GetCurrentThreadId();
}

void LockProfiler::OnLockReleased(void* lockAddr) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    lockOwners_.erase(lockAddr);
}

void LockProfiler::OnLockContended(void* lockAddr, long long waitTime) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    LockInfo& info = lockData_[lockAddr];
    info.contentionCount++;
    info.totalWaitTime += waitTime;

    auto stack = GetStackTrace();
    std::string stackStr;
    for (const auto& frame : stack) {
        if (!stackStr.empty()) {
            stackStr += ";";
        }
        stackStr += frame;
    }
    info.stackTraces[stackStr]++;
}

void LockProfiler::OutputFlameGraph(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return;
    }

    std::lock_guard<std::mutex> lock(dataMutex_);

    file << "[\n";
    bool first = true;

    for (const auto& pair : lockData_) {
        const LockInfo& info = pair.second;

        for (const auto& stackPair : info.stackTraces) {
            if (!first) {
                file << ",\n";
            }
            first = false;

            file << "{\"name\": \"" << stackPair.first << "\",\"value\": " << stackPair.second << "}";
        }
    }

    file << "\n]";
    file.close();
}

void LockProfiler::SamplingThread() {
    while (isRunning_) {
        std::this_thread::sleep_for(std::chrono::microseconds(1000)); // 1000Hz

        std::lock_guard<std::mutex> lock(dataMutex_);
        // 采样逻辑可以在这里扩展
    }
}

std::vector<std::string> LockProfiler::GetStackTrace() {
    return StackWalker::GetCallStack();
}