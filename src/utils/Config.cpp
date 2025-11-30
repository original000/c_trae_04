#include "Config.h"
#include <fstream>
#include <sstream>
#include <iostream>

Config::Config() {
}

Config::~Config() {
}

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

bool Config::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Remove comments
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }

        // Trim whitespace
        size_t start_pos = line.find_first_not_of(" \t\n\r");
        if (start_pos == std::string::npos) {
            continue; // Empty line
        }

        size_t end_pos = line.find_last_not_of(" \t\n\r");
        line = line.substr(start_pos, end_pos - start_pos + 1);

        // Split key and value
        size_t equals_pos = line.find('=');
        if (equals_pos == std::string::npos) {
            std::cerr << "Invalid config line: " << line << std::endl;
            continue;
        }

        std::string key = line.substr(0, equals_pos);
        std::string value = line.substr(equals_pos + 1);

        // Trim whitespace from key and value
        start_pos = key.find_first_not_of(" \t\n\r");
        end_pos = key.find_last_not_of(" \t\n\r");
        key = key.substr(start_pos, end_pos - start_pos + 1);

        start_pos = value.find_first_not_of(" \t\n\r");
        end_pos = value.find_last_not_of(" \t\n\r");
        value = value.substr(start_pos, end_pos - start_pos + 1);

        // Add to config map
        config_map_[key] = value;
    }

    file.close();
    return true;
}

std::string Config::get(const std::string& key, const std::string& default_value) const {
    std::lock_guard<std::mutex> lock(mutex_);

    if (config_map_.find(key) != config_map_.end()) {
        return config_map_[key];
    }

    return default_value;
}

int Config::getInt(const std::string& key, int default_value) const {
    std::string value = get(key);
    if (value.empty()) {
        return default_value;
    }

    try {
        return std::stoi(value);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse int config value for key '" << key << "': " << e.what() << std::endl;
        return default_value;
    }
}

bool Config::getBool(const std::string& key, bool default_value) const {
    std::string value = get(key);
    if (value.empty()) {
        return default_value;
    }

    if (value == "true" || value == "1" || value == "yes") {
        return true;
    } else if (value == "false" || value == "0" || value == "no") {
        return false;
    } else {
        std::cerr << "Failed to parse bool config value for key '" << key << "': " << value << std::endl;
        return default_value;
    }
}

double Config::getDouble(const std::string& key, double default_value) const {
    std::string value = get(key);
    if (value.empty()) {
        return default_value;
    }

    try {
        return std::stod(value);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse double config value for key '" << key << "': " << e.what() << std::endl;
        return default_value;
    }
}

void Config::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);

    config_map_[key] = value;
}

void Config::setInt(const std::string& key, int value) {
    set(key, std::to_string(value));
}

void Config::setBool(const std::string& key, bool value) {
    set(key, value ? "true" : "false");
}

void Config::setDouble(const std::string& key, double value) {
    set(key, std::to_string(value));
}

void Config::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    config_map_.erase(key);
}

void Config::clear() {
    std::lock_guard<std::mutex> lock(mutex_);

    config_map_.clear();
}

bool Config::contains(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);

    return config_map_.find(key) != config_map_.end();
}

int Config::size() const {
    std::lock_guard<std::mutex> lock(mutex_);

    return config_map_.size();
}
