#include <iostream>
#include <signal.h>
#include "src/server/HttpServer.h"
#include "src/utils/Config.h"
#include "src/utils/Logger.h"

// Global server instance (needed for signal handling)
HttpServer* g_server = nullptr;

// Signal handler function
void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", stopping server...\n";

    if (g_server) {
        g_server->stop();
    }

    exit(signum);
}

int main() {
    try {
        // Load configuration
        Config::getInstance().load();

        // Initialize logger
        Logger::getInstance().init("shortener.log");
        LOG_INFO("Starting Short Link Service...");

        // Create and initialize the server
        HttpServer server;
        g_server = &server;

        if (!server.init()) {
            LOG_ERROR("Failed to initialize server");
            return 1;
        }

        // Set up signal handlers for graceful shutdown
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        // Start the server
        server.start();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
        LOG_ERROR(std::string("Failed to start server: ") + e.what());
        // 等待用户输入，以便查看错误信息
        std::cout << "Press any key to exit..." << std::endl;
        std::cin.get();
        return 1;
    }
}
