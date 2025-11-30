#ifndef LOGGER_H
#define LOGGER_H

#include "LogStream.h"
#include "AsyncLogger.h"
#include <chrono>
#include <string>

enum class LogLevel {
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    Logger(LogLevel level);
    ~Logger();

    LogStream& stream() {
        return stream_;
    }

private:
    std::string getTime();
    std::string getLevelName(LogLevel level);

    LogLevel level_;
    LogStream stream_;
};

// Global async logger instance
extern AsyncLogger* g_asyncLogger;

// Log macros
#define LOG_INFO Logger(LogLevel::INFO).stream()
#define LOG_WARN Logger(LogLevel::WARN).stream()
#define LOG_ERROR Logger(LogLevel::ERROR).stream()

#endif // LOGGER_H
