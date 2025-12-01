#include "MemLeakDetector.h"
#include <iostream>

int main() {
    std::cout << "Simple Memory Leak Detector Test" << std::endl;
    std::cout << "======================================" << std::endl;
    
    // 制造1处泄漏
    int* leak = new int;
    *leak = 42;
    
    // 制造一些不会泄漏的内存分配和释放
    int* normal = new int;
    *normal = 100;
    delete normal;
    
    std::cout << "Test completed. Checking for memory leaks..." << std::endl;
    
    return 0;
}