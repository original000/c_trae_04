#ifndef LOCKPROFILER_H
#define LOCKPROFILER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <windows.h>

struct LockInfo {
    std::atomic<int> contentionCount;
    std::atomic<long long> totalWaitTime;
    std::unordered_map<std::string, int> stackTraces;

    LockInfo() : contentionCount(0), totalWaitTime(0) {}
};

class LockProfiler {
public:
    static LockProfiler& Instance() {
        static LockProfiler instance;
        return instance;
    }

    void Start();
    void Stop();
    bool IsRunning() const;
    void OnLockAcquired(void* lockAddr);
    void OnLockReleased(void* lockAddr);
    void OnLockContended(void* lockAddr, long long waitTime);
    void OutputFlameGraph(const std::string& filename);

private:
    LockProfiler();
    ~LockProfiler();

    void SamplingThread();
    std::vector<std::string> GetStackTrace();

    friend class LockProfilerInstance;
    std::atomic<bool> isRunning_;
    std::thread samplingThread_;
    std::mutex dataMutex_;
    std::unordered_map<void*, LockInfo> lockData_;
    std::unordered_map<void*, DWORD> lockOwners_;
};

// 辅助宏#define LOCK_PROFILER LockProfiler::Instance()

#endif // LOCKPROFILER_H