#include "ConfigManager.h"
#include <iostream>
#include <vector>
#include <string>
#include <thread>

// 配置变化回调函数
void on_config_change() {
    std::cout << "\nConfig changed! New values: " << std::endl;
    
    // 读取配置值
    int int_value = ConfigManager::instance().get<int>("server.port", 8080);
    double double_value = ConfigManager::instance().get<double>("server.timeout", 30.5);
    std::string string_value = ConfigManager::instance().get<std::string>("server.name", "DefaultServer");
    bool bool_value = ConfigManager::instance().get<bool>("server.enabled", false);
    std::vector<int> int_array = ConfigManager::instance().get<std::vector<int>>("server.ports", {80, 443});
    std::vector<std::string> string_array = ConfigManager::instance().get<std::vector<std::string>>("server.hosts", {"localhost"});
    
    // 打印配置值
    std::cout << "server.port: " << int_value << std::endl;
    std::cout << "server.timeout: " << double_value << std::endl;
    std::cout << "server.name: " << string_value << std::endl;
    std::cout << "server.enabled: " << std::boolalpha << bool_value << std::endl;
    
    std::cout << "server.ports: [";
    for (size_t i = 0; i < int_array.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << int_array[i];
    }
    std::cout << "]" << std::endl;
    
    std::cout << "server.hosts: [";
    for (size_t i = 0; i < string_array.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << string_array[i];
    }
    std::cout << "]" << std::endl;
}

int main() {
    // 加载配置文件
    if (!ConfigManager::instance().load("default.json", "dev.json")) {
        std::cerr << "Failed to load config files" << std::endl;
        return 1;
    }
    
    // 注册配置变化回调
    ConfigManager::instance().register_callback(on_config_change);
    
    // 启动文件监控（5秒间隔）
    ConfigManager::instance().start_watch(5);
    
    std::cout << "Config manager started. Press Ctrl+C to exit." << std::endl;
    
    // 初始打印配置值
    on_config_change();
    
    // 保持程序运行
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}