#pragma once

#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>

class Logger {
public:
    enum Level {
        DEBUG,
        INFO,
        WARNING,
        LOG_ERROR
    };

    static Logger& getInstance();
    void init(const std::string& filename);
    void log(Level level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void stop();

private:
    Logger();
    ~Logger();

    std::string getTimestamp();
    std::string levelToString(Level level);
    void processQueue();

    std::queue<std::tuple<std::string, std::string, std::string>> queue_;
    std::mutex mutex_;
    std::condition_variable cond_var_;
    std::thread worker_thread_;
    std::ofstream log_file_;
    bool stop_;
};

#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
