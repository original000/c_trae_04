#include <iostream>
#include <string>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "src/Database.h"
#include "src/Command.h"
#include "src/AofPersistence.h"
#include "src/HttpServer.h"

int main() {
    // 打印g++版本和Windows检测
    std::cout << "=== MiniRedis Memory Database ===\n";
    std::cout << "g++ version: ";
    system("g++ --version");
    
#ifdef _WIN32
    std::cout << "Operating System: Windows\n";
#else
    std::cout << "Operating System: Linux/Unix\n";
#endif
    std::cout << "==================================\n\n";

    // 初始化数据库
    Database db;

    // 初始化AOF持久化
    AofPersistence aof(db);

    // 从AOF文件加载数据
    std::cout << "Loading data from AOF file...\n";
    if (aof.load()) {
        std::cout << "Data loaded successfully\n";
    } else {
        std::cout << "Failed to load data from AOF file\n";
    }

    // 启动AOF持久化
    if (aof.start()) {
        std::cout << "AOF persistence started\n";
    } else {
        std::cout << "Failed to start AOF persistence\n";
        return 1;
    }

    // 启动HTTP服务器
    HttpServer http_server(db, aof, 8080);
    if (http_server.start()) {
        std::cout << "HTTP server started\n";
    } else {
        std::cout << "Failed to start HTTP server\n";
        aof.stop();
        return 1;
    }

    // 等待用户输入退出命令
    std::cout << "\nMiniRedis is running. Press 'q' and enter to quit.\n";
    char quit; 
    while (std::cin >> quit) {
        if (quit == 'q' || quit == 'Q') {
            break;
        }
    }

    // 停止服务器和持久化
    std::cout << "Shutting down...\n";
    http_server.stop();
    aof.stop();

    std::cout << "MiniRedis has been stopped.\n";

    return 0;
}
