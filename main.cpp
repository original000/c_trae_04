#include <iostream>
#include <string>
#include "Config.h"
#include "Snowflake.h"
#include "HttpServer.h"

// 打印编译环境信息
void printEnvironmentInfo() {
    std::cout << "==========================================\n";
    std::cout << "Snowflake ID Generator - Environment Info\n";
    std::cout << "==========================================\n";
    
    // 打印g++版本
    std::cout << "g++ Version: ";
    system("g++ --version");
    
    // 打印操作系统
    std::cout << "Operating System: Windows\n";
    std::cout << "==========================================\n\n";
}

int main(int argc, char* argv[]) {
    try {
        // 打印环境信息
        printEnvironmentInfo();
        
        // 加载配置文件
        std::string config_file = "config.json";
        if (argc > 1) {
            config_file = argv[1];
        }
        
        Config config(config_file);
        int worker_id = config.getWorkerId();
        int datacenter_id = config.getDatacenterId();
        
        // 打印workerId
        std::cout << "Worker ID: " << worker_id << "\n";
        std::cout << "Datacenter ID: " << datacenter_id << "\n\n";
        
        // 初始化雪花算法生成器
        Snowflake snowflake(worker_id, datacenter_id);
        
        // 启动HTTP服务器
        int port = 10010;
        HttpServer server(port, snowflake);
        server.start();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
