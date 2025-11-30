#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include "json.hpp"

using json = nlohmann::json;

class ConfigManager {
public:
    // 单例模式
    static ConfigManager& instance();
    
    // 禁止拷贝和移动
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(ConfigManager&&) = delete;
    
    // 加载配置文件
    bool load(const std::string& default_path, const std::string& env_path);
    
    // 类型安全的配置读取
    template <typename T>
    T get(const std::string& path, const T& default_value = T()) const;
    
    // 类型安全的配置读取（vector 特化）
    template <typename T>
    std::vector<T> get(const std::string& path, const std::vector<T>& default_value) const;
    
    // 启动文件监控
    void start_watch(int interval_seconds = 5);
    
    // 停止文件监控
    void stop_watch();
    
    // 注册配置变化回调
    using ConfigChangeCallback = std::function<void()>;
    void register_callback(ConfigChangeCallback callback);
    
private:
    ConfigManager();
    ~ConfigManager();
    
    // 合并配置
    void merge_config(json& target, const json& source);
    
    // 从路径获取JSON值
    const json* get_json_by_path(const std::string& path) const;
    
    // 检查文件是否修改
    bool is_file_modified(const std::string& file_path) const;
    
    // 监控线程函数
    void watch_thread();
    
    json config_;  // 合并后的配置
    
    std::string default_path_;
    std::string env_path_;
    
    // 文件最后修改时间
    std::map<std::string, std::chrono::system_clock::time_point> file_last_modified_;
    
    bool watch_running_;  // 监控线程是否运行
    int watch_interval_;   // 监控间隔（秒）
    
    std::vector<ConfigChangeCallback> callbacks_;  // 配置变化回调
};

// 模板函数实现
#include "ConfigManager.tpp"

#endif // CONFIGMANAGER_H