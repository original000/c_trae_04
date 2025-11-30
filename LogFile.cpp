#include "LogFile.h"
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <vector>
#include <ctime>

LogFile::LogFile(const std::string& basename)
    : basename_(basename),
      fileSize_(0)
{
    rollFile();
}

LogFile::~LogFile()
{
    flush();
    if (file_.is_open()) {
        file_.close();
    }
}

void LogFile::append(const char* data, int len)
{
    file_.write(data, len);
    fileSize_ += len;

    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm nowTm = *std::localtime(&nowTime);

    auto lastRollTime = std::chrono::system_clock::to_time_t(lastRoll_);
    std::tm lastRollTm = *std::localtime(&lastRollTime);

    // Roll if file size exceeds limit or it's a new day
    if (fileSize_ >= kRollSize_ || nowTm.tm_mday != lastRollTm.tm_mday) {
        rollFile();
    }

    // Flush every 3 seconds
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastFlush_).count() >= 3) {
        flush();
        lastFlush_ = now;
    }
}

void LogFile::flush()
{
    if (file_.is_open()) {
        file_.flush();
    }
}

void LogFile::rollFile()
{
    if (file_.is_open()) {
        file_.close();
    }

    auto now = std::chrono::system_clock::now();
    std::string filename = getLogFileName(basename_, now);
    file_.open(filename, std::ios::app | std::ios::binary);

    lastRoll_ = now;
    lastFlush_ = now;
    fileSize_ = 0;

    deleteOldFiles();
}

void LogFile::deleteOldFiles()
{
    std::vector<std::string> files;
    DIR* dir = opendir(".");
    if (dir == nullptr) {
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        if (filename.find(basename_ + ".log.") == 0) {
            files.push_back(filename);
        }
    }
    closedir(dir);

    // Sort files by modification time (oldest first)
    std::sort(files.begin(), files.end(), [](const std::string& a, const std::string& b) {
        struct stat statA, statB;
        stat(a.c_str(), &statA);
        stat(b.c_str(), &statB);
        return statA.st_mtime < statB.st_mtime;
    });

    // Delete files older than 7 days
    time_t now; 
    time(&now);
    time_t sevenDaysAgo = now - (7 * 24 * 60 * 60);

    for (const auto& file : files) {
        struct stat statFile;
        stat(file.c_str(), &statFile);
        if (statFile.st_mtime < sevenDaysAgo) {
            remove(file.c_str());
        }
    }
}

std::string LogFile::getLogFileName(const std::string& basename, const std::chrono::system_clock::time_point& tp)
{
    char buf[256];
    auto time = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&time);
    snprintf(buf, sizeof(buf), "%s.log.%04d%02d%02d", basename.c_str(), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    return std::string(buf);
}
