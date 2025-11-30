#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include <string>
#include <sstream>
#include <cstdint>

class LogStream {
public:
    LogStream() = default;
    ~LogStream() = default;

    // Overload << for all basic types
    LogStream& operator<<(bool val) {
        oss_ << (val ? "true" : "false");
        return *this;
    }

    LogStream& operator<<(char val) {
        oss_ << val;
        return *this;
    }

    LogStream& operator<<(signed char val) {
        oss_ << static_cast<char>(val);
        return *this;
    }

    LogStream& operator<<(unsigned char val) {
        oss_ << static_cast<char>(val);
        return *this;
    }

    LogStream& operator<<(short val) {
        oss_ << val;
        return *this;
    }

    LogStream& operator<<(unsigned short val) {
        oss_ << val;
        return *this;
    }

    LogStream& operator<<(int val) {
        oss_ << val;
        return *this;
    }

    LogStream& operator<<(unsigned int val) {
        oss_ << val;
        return *this;
    }

    LogStream& operator<<(long val) {
        oss_ << val;
        return *this;
    }

    LogStream& operator<<(unsigned long val) {
        oss_ << val;
        return *this;
    }

    LogStream& operator<<(long long val) {
        oss_ << val;
        return *this;
    }

    LogStream& operator<<(unsigned long long val) {
        oss_ << val;
        return *this;
    }

    LogStream& operator<<(float val) {
        oss_ << val;
        return *this;
    }

    LogStream& operator<<(double val) {
        oss_ << val;
        return *this;
    }

    LogStream& operator<<(long double val) {
        oss_ << val;
        return *this;
    }

    LogStream& operator<<(const char* str) {
        if (str) {
            oss_ << str;
        } else {
            oss_ << "(null)";
        }
        return *this;
    }

    LogStream& operator<<(const std::string& str) {
        oss_ << str;
        return *this;
    }

    // Get the string content
    std::string str() const {
        return oss_.str();
    }

    // Clear the stream
    void clear() {
        oss_.str("");
        oss_.clear();
    }

private:
    std::ostringstream oss_;
};

#endif // LOGSTREAM_H
