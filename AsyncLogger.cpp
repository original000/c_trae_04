#include "AsyncLogger.h"
#include <cstring>
#include <chrono>
#include <iostream>

AsyncLogger::AsyncLogger(const std::string& basename)
    : basename_(basename),
      running_(false),
      head_(0),
      tail_(0),
      itemSize_(sizeof(LogItem) + 1024), // 1024 bytes per log item
      logFile_(basename)
{
    buffer_.resize(kQueueSize);
    for (int i = 0; i < kQueueSize; ++i) {
        buffer_[i] = new char[itemSize_];
    }
}

AsyncLogger::~AsyncLogger()
{
    stop();
    for (int i = 0; i < kQueueSize; ++i) {
        delete[] buffer_[i];
    }
}

void AsyncLogger::start()
{
    running_ = true;
    thread_ = std::thread(&AsyncLogger::threadFunc, this);
}

void AsyncLogger::stop()
{
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

void AsyncLogger::append(const char* logline, int len)
{
    if (len + sizeof(LogItem) > itemSize_) {
        len = itemSize_ - sizeof(LogItem) - 1;
    }

    int currentTail = tail_.load(std::memory_order_relaxed);
    int nextTail = (currentTail + 1) % kQueueSize;

    while (nextTail == head_.load(std::memory_order_acquire)) {
        // Queue is full, spin wait
    }

    LogItem* item = reinterpret_cast<LogItem*>(buffer_[currentTail]);
    item->len = len;
    memcpy(item->data, logline, len);
    item->data[len] = '\0';

    tail_.store(nextTail, std::memory_order_release);
}

void AsyncLogger::threadFunc()
{
    while (running_) {
        int currentHead = head_.load(std::memory_order_relaxed);
        int currentTail = tail_.load(std::memory_order_acquire);

        while (currentHead != currentTail) {
            LogItem* item = reinterpret_cast<LogItem*>(buffer_[currentHead]);
            logFile_.append(item->data, item->len);

            currentHead = (currentHead + 1) % kQueueSize;
            head_.store(currentHead, std::memory_order_release);
        }

        logFile_.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Flush remaining logs
    int currentHead = head_.load(std::memory_order_relaxed);
    int currentTail = tail_.load(std::memory_order_acquire);

    while (currentHead != currentTail) {
        LogItem* item = reinterpret_cast<LogItem*>(buffer_[currentHead]);
        logFile_.append(item->data, item->len);

        currentHead = (currentHead + 1) % kQueueSize;
        head_.store(currentHead, std::memory_order_release);
    }

    logFile_.flush();
}
