#include "Config.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

// 简单的JSON解析函数
std::string getValueFromJSON(const std::string& json, const std::string& key) {
    size_t key_pos = json.find('"' + key + '"');
    if (key_pos == std::string::npos) {
        throw std::runtime_error("Key not found: " + key);
    }

    size_t colon_pos = json.find(':', key_pos);
    if (colon_pos == std::string::npos) {
        throw std::runtime_error("Invalid JSON format for key: " + key);
    }

    size_t value_start = json.find_first_not_of(" \t\n\r", colon_pos + 1);
    if (value_start == std::string::npos) {
        throw std::runtime_error("No value found for key: " + key);
    }

    size_t value_end;
    if (json[value_start] == '"') {
        // 字符串值
        value_start++;
        value_end = json.find('"', value_start);
        if (value_end == std::string::npos) {
            throw std::runtime_error("Unterminated string value for key: " + key);
        }
    } else {
        // 数字值
        value_end = json.find_first_of(",}\n\r\t", value_start);
        if (value_end == std::string::npos) {
            value_end = json.size();
        }
    }

    return json.substr(value_start, value_end - value_start);
}

Config::Config(const std::string& config_file) {
    parseConfig(config_file);
}

int Config::getWorkerId() const {
    return worker_id_;
}

int Config::getDatacenterId() const {
    return datacenter_id_;
}

void Config::parseConfig(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + config_file);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_content = buffer.str();

    try {
        std::string worker_id_str = getValueFromJSON(json_content, "workerId");
        std::string datacenter_id_str = getValueFromJSON(json_content, "datacenterId");

        worker_id_ = std::stoi(worker_id_str);
        datacenter_id_ = std::stoi(datacenter_id_str);

        // 验证workerId和datacenterId是否在有效范围内（0-31）
        if (worker_id_ < 0 || worker_id_ > 31) {
            throw std::runtime_error("workerId must be between 0 and 31");
        }
        if (datacenter_id_ < 0 || datacenter_id_ > 31) {
            throw std::runtime_error("datacenterId must be between 0 and 31");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse config file: " + std::string(e.what()));
    }
}
