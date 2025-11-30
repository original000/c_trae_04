#include "ConfigManager.h"
#include <fstream>
#include <thread>
#include <algorithm>
#include <iostream>
#include <windows.h>
#include <sys/stat.h>

ConfigManager::ConfigManager() 
    : watch_running_(false), watch_interval_(5) {
}

ConfigManager::~ConfigManager() {
    stop_watch();
}

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::load(const std::string& default_path, const std::string& env_path) {
    default_path_ = default_path;
    env_path_ = env_path;
    
    json default_config;
    json env_config;
    
    // 加载默认配置
    if (!default_path.empty()) {
        std::ifstream default_file(default_path);
        if (default_file.is_open()) {
            try {
                default_file >> default_config;
                
                // 获取文件最后修改时间
                struct stat file_stat;
                if (stat(default_path.c_str(), &file_stat) == 0) {
                    file_last_modified_[default_path] = std::chrono::system_clock::from_time_t(
                        file_stat.st_mtime
                    );
                }
            } catch (const std::exception& e) {
                std::cerr << "Failed to parse default config: " << e.what() << std::endl;
                return false;
            }
        } else {
            std::cerr << "Failed to open default config file: " << default_path << std::endl;
            return false;
        }
    }
    
    // 加载环境配置
    if (!env_path.empty()) {
        std::ifstream env_file(env_path);
        if (env_file.is_open()) {
            try {
                env_file >> env_config;
                
                // 获取文件最后修改时间
                struct stat file_stat;
                if (stat(env_path.c_str(), &file_stat) == 0) {
                    file_last_modified_[env_path] = std::chrono::system_clock::from_time_t(
                        file_stat.st_mtime
                    );
                }
            } catch (const std::exception& e) {
                std::cerr << "Failed to parse env config: " << e.what() << std::endl;
                return false;
            }
        } else {
            std::cerr << "Failed to open env config file: " << env_path << std::endl;
            return false;
        }
    }
    
    // 合并配置：环境配置覆盖默认配置
    config_ = default_config;
    merge_config(config_, env_config);
    
    return true;
}

void ConfigManager::merge_config(json& target, const json& source) {
    if (!source.is_object()) {
        return;
    }
    
    for (const auto& item : source.items()) {
        const std::string& key = item.key();
        const json& value = item.value();
        
        if (target.contains(key) && target[key].is_object() && value.is_object()) {
            // 递归合并对象
            merge_config(target[key], value);
        } else {
            // 覆盖或添加新值
            target[key] = value;
        }
    }
}

const json* ConfigManager::get_json_by_path(const std::string& path) const {
    if (path.empty()) {
        return &config_;
    }
    
    const json* current = &config_;
    std::string::size_type start = 0;
    
    while (true) {
        std::string::size_type dot_pos = path.find('.', start);
        std::string key = path.substr(start, dot_pos - start);
        
        if (!current->is_object() || !current->contains(key)) {
            return nullptr;
        }
        
        current = &((*current)[key]);
        
        if (dot_pos == std::string::npos) {
            break;
        }
        
        start = dot_pos + 1;
    }
    
    return current;
}

void ConfigManager::start_watch(int interval_seconds) {
    if (watch_running_) {
        return;
    }
    
    watch_interval_ = interval_seconds;
    watch_running_ = true;
    
    // 启动监控线程
    std::thread([this]() {
        while (watch_running_) {
            bool modified = false;
            
            // 检查默认配置文件
            if (!default_path_.empty() && is_file_modified(default_path_)) {
                modified = true;
            }
            
            // 检查环境配置文件
            if (!env_path_.empty() && is_file_modified(env_path_)) {
                modified = true;
            }
            
            // 如果文件有修改，重新加载配置
            if (modified) {
                std::cout << "Config file modified, reloading..." << std::endl;
                if (load(default_path_, env_path_)) {
                    // 触发所有回调
                    for (auto& callback : callbacks_) {
                        callback();
                    }
                } else {
                    std::cerr << "Failed to reload config" << std::endl;
                }
            }
            
            // 等待指定间隔
            std::this_thread::sleep_for(std::chrono::seconds(watch_interval_));
        }
    }).detach();
}

void ConfigManager::stop_watch() {
    watch_running_ = false;
}

void ConfigManager::register_callback(ConfigChangeCallback callback) {
    callbacks_.push_back(callback);
}

bool ConfigManager::is_file_modified(const std::string& file_path) const {
    try {
        struct stat file_stat;
        if (stat(file_path.c_str(), &file_stat) == 0) {
            auto current_time = std::chrono::system_clock::from_time_t(file_stat.st_mtime);
            
            auto it = file_last_modified_.find(file_path);
            if (it == file_last_modified_.end() || current_time > it->second) {
                // 更新最后修改时间
                const_cast<ConfigManager*>(this)->file_last_modified_[file_path] = current_time;
                return true;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to check file modification: " << e.what() << std::endl;
    }
    
    return false;
}