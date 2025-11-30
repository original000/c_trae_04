#pragma once

#include <string>

class Config {
public:
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    void load() {
        // For simplicity, hardcode config values here
        // In a real application, this would load from a file
        server_address_ = "0.0.0.0";
        server_port_ = 8081;
        database_path_ = "shortener.db";
        cache_capacity_ = 10000;
        thread_pool_size_ = 16;
    }

    const std::string& getServerAddress() const {
        return server_address_;
    }

    int getServerPort() const {
        return server_port_;
    }

    const std::string& getDatabasePath() const {
        return database_path_;
    }

    int getCacheCapacity() const {
        return cache_capacity_;
    }

    int getThreadPoolSize() const {
        return thread_pool_size_;
    }

private:
    Config() {}

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::string server_address_;
    int server_port_;
    std::string database_path_;
    int cache_capacity_;
    int thread_pool_size_;
};
