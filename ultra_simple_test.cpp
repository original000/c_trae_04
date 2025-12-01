#include "MemLeakDetector.h"
#include <iostream>
#include <exception>

int main() {
    try {
        std::cout << "Ultra Simple Memory Leak Detector Test" << std::endl;
        std::cout << "============================================" << std::endl;
        
        // 制造1处泄漏
        int* leak = new int;
        *leak = 42;
        
        std::cout << "Test completed. Checking for memory leaks..." << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "Unknown exception caught" << std::endl;
        return 1;
    }
}