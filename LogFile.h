#ifndef LOGFILE_H
#define LOGFILE_H

#include <string>
#include <fstream>
#include <chrono>

class LogFile {
public:
    LogFile(const std::string& basename);
    ~LogFile();

    void append(const char* data, int len);
    void flush();

private:
    void rollFile();
    void deleteOldFiles();
    std::string getLogFileName(const std::string& basename, const std::chrono::system_clock::time_point& tp);

    std::string basename_;
    std::ofstream file_;
    std::chrono::system_clock::time_point lastRoll_;
    std::chrono::system_clock::time_point lastFlush_;
    long long fileSize_;
    const long long kRollSize_ = 100 * 1024 * 1024; // 100MB
};

#endif // LOGFILE_H
