#include "Logger.h"
#include <thread>
#include <vector>
#include <iostream>
#include <cstdlib>

int main() {
    // Print GCC version
    std::cout << "GCC version: ";
    system("g++ --version");
    std::cout << std::endl;

    // Initialize async logger
    AsyncLogger logger("test_log");
    g_asyncLogger = &logger;
    logger.start();

    std::cout << "Starting 8 threads to log..." << std::endl;

    // Create 8 threads to log messages
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([i]() {
            for (int j = 0; j < 10000; ++j) {
                LOG_INFO << "Thread " << i << ": Log message " << j;
                if (j % 100 == 0) {
                    LOG_WARN << "Thread " << i << ": Warning message " << j;
                }
                if (j % 1000 == 0) {
                    LOG_ERROR << "Thread " << i << ": Error message " << j;
                }
            }
        });
    }

    // Join all threads
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "All threads finished logging." << std::endl;

    // Stop logger
    logger.stop();

    return 0;
}
