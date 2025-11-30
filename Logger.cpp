#include "Logger.h"
#include <cstdio>
#include <ctime>

// Initialize global logger pointer
AsyncLogger* g_asyncLogger = nullptr;

Logger::Logger(LogLevel level)
    : level_(level)
{
    stream_ << getTime() << " [" << getLevelName(level_) << "] ";
}

Logger::~Logger()
{
    stream_ << "\n";
    std::string logStr = stream_.str();
    if (g_asyncLogger) {
        g_asyncLogger->append(logStr.c_str(), logStr.size());
    } else {
        // Fallback to stdout if async logger not initialized
        fwrite(logStr.c_str(), 1, logStr.size(), stdout);
        fflush(stdout);
    }
}

std::string Logger::getTime()
{
    char buf[32];
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    // Get milliseconds
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec, static_cast<int>(ms.count()));

    return std::string(buf);
}

std::string Logger::getLevelName(LogLevel level)
{
    switch (level) {
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}
