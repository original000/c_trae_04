#include "ConfigManager.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

void test_config_manager() {
    std::cout << "Starting config manager tests..." << std::endl;
    
    // 测试1：加载配置文件
    std::cout << "Test 1: Loading config files..." << std::endl;
    if (!ConfigManager::instance().load("default.json", "dev.json")) {
        std::cerr << "Test 1 failed: Failed to load config files" << std::endl;
        return;
    }
    std::cout << "Test 1 passed: Config files loaded successfully" << std::endl;
    
    // 测试2：读取基本类型配置
    std::cout << "Test 2: Reading basic type configs..." << std::endl;
    
    int port = ConfigManager::instance().get<int>("server.port", 8080);
    assert(port == 8081);
    
    double timeout = ConfigManager::instance().get<double>("server.timeout", 30.0);
    assert(timeout == 60.0);
    
    std::string name = ConfigManager::instance().get<std::string>("server.name", "DefaultServer");
    assert(name == "DevServer");
    
    bool enabled = ConfigManager::instance().get<bool>("server.enabled", false);
    assert(enabled == true);
    
    std::cout << "Test 2 passed: All basic type configs read correctly" << std::endl;
    
    // 测试3：读取vector类型配置
    std::cout << "Test 3: Reading vector type configs..." << std::endl;
    
    std::vector<int> ports = ConfigManager::instance().get<std::vector<int>>("server.ports", {80, 443});
    assert(ports.size() == 3);
    assert(ports[0] == 8080);
    assert(ports[1] == 8081);
    assert(ports[2] == 8082);
    
    std::vector<std::string> hosts = ConfigManager::instance().get<std::vector<std::string>>("server.hosts", {"localhost"});
    assert(hosts.size() == 2);
    assert(hosts[0] == "localhost");
    assert(hosts[1] == "dev.example.com");
    
    std::cout << "Test 3 passed: All vector type configs read correctly" << std::endl;
    
    // 测试4：读取不存在的配置
    std::cout << "Test 4: Reading non-existent configs..." << std::endl;
    
    int non_existent_int = ConfigManager::instance().get<int>("server.non_existent", 123);
    assert(non_existent_int == 123);
    
    std::string non_existent_string = ConfigManager::instance().get<std::string>("server.non_existent", "default");
    assert(non_existent_string == "default");
    
    std::vector<int> non_existent_vector = ConfigManager::instance().get<std::vector<int>>("server.non_existent", {1, 2, 3});
    assert(non_existent_vector.size() == 3);
    assert(non_existent_vector[0] == 1);
    assert(non_existent_vector[1] == 2);
    assert(non_existent_vector[2] == 3);
    
    std::cout << "Test 4 passed: All non-existent configs returned default values correctly" << std::endl;
    
    // 测试5：读取数据库配置
    std::cout << "Test 5: Reading database configs..." << std::endl;
    
    std::string db_host = ConfigManager::instance().get<std::string>("database.host", "default_host");
    assert(db_host == "localhost");
    
    int db_port = ConfigManager::instance().get<int>("database.port", 1234);
    assert(db_port == 3306);
    
    std::string db_username = ConfigManager::instance().get<std::string>("database.username", "default_user");
    assert(db_username == "root");
    
    std::string db_password = ConfigManager::instance().get<std::string>("database.password", "default_pass");
    assert(db_password == "password");
    
    std::string db_database = ConfigManager::instance().get<std::string>("database.database", "default_db");
    assert(db_database == "dev_db");
    
    std::cout << "Test 5 passed: All database configs read correctly" << std::endl;
    
    std::cout << "All tests passed!" << std::endl;
}

int main() {
    test_config_manager();
    return 0;
}