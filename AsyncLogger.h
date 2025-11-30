#ifndef ASYNCLOGGER_H
#define ASYNCLOGGER_H

#include <atomic>
#include <thread>
#include <vector>
#include <functional>
#include "LogFile.h"

const int kQueueSize = 8192;

class AsyncLogger {
public:
    AsyncLogger(const std::string& basename);
    ~AsyncLogger();

    void append(const char* logline, int len);
    void start();
    void stop();

private:
    void threadFunc();

    struct LogItem {
        int len;
        char data[0];
    };

    std::string basename_;
    std::thread thread_;
    std::atomic<bool> running_;

    // MPSC lock-free queue
    std::atomic<int> head_;
    std::atomic<int> tail_;
    std::vector<char*> buffer_;
    int itemSize_;

    LogFile logFile_;
};

#endif // ASYNCLOGGER_H
