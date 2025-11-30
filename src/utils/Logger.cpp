#include "Logger.h"
#include <chrono>
#include <sstream>
#include <iomanip>

Logger::Logger() : stop_(false) {
}

Logger::~Logger() {
    stop();
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::init(const std::string& filename) {
    log_file_.open(filename, std::ios::app);
    if (!log_file_.is_open()) {
        throw std::runtime_error("Failed to open log file");
    }

    worker_thread_ = std::thread(&Logger::processQueue, this);
}

void Logger::log(Level level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.emplace(getTimestamp(), levelToString(level), message);
    cond_var_.notify_one();
}

void Logger::debug(const std::string& message) {
    log(DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(INFO, message);
}

void Logger::warning(const std::string& message) {
    log(WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LOG_ERROR, message);
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::levelToString(Level level) 
{
    switch (level) 
    {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case LOG_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

void Logger::processQueue() {
    while (true) {
        std::tuple<std::string, std::string, std::string> entry;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_var_.wait(lock, [this]() { return !queue_.empty() || stop_; });

            if (stop_ && queue_.empty()) {
                break;
            }

            entry = std::move(queue_.front());
            queue_.pop();
        }

        if (log_file_.is_open()) {
            log_file_ << std::get<0>(entry) << " [" << std::get<1>(entry) << "] " << std::get<2>(entry) << std::endl;
        }
    }
}

void Logger::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }

    cond_var_.notify_one();

    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }

    if (log_file_.is_open()) {
        log_file_.close();
    }
}
