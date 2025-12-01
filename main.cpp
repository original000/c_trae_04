#include "MemLeakDetector.h"
#include <iostream>
#include <vector>

// 用于制造泄漏的函数
void createLeak() {
    int* leak = new int; // 制造1处泄漏
    *leak = 42;
}

// 用于制造数组泄漏的函数
void createArrayLeak() {
    char* leak = new char[100]; // 制造1处数组泄漏
    leak[0] = 'a';
}

// 用于制造双重释放的函数
void createDoubleFree() {
    int* ptr = new int;
    delete ptr;
    delete ptr; // 制造1次双重释放
}

// 用于制造数组双重释放的函数
void createArrayDoubleFree() {
    int* ptr = new int[10];
    delete[] ptr;
    delete[] ptr; // 制造1次数组双重释放
}

int main() {
    std::cout << "Memory Leak Detector Test" << std::endl;
    std::cout << "==========================" << std::endl;
    
    // 制造10处泄漏：5个int泄漏和5个数组泄漏
    for (int i = 0; i < 5; ++i) {
        createLeak();
    }
    
    for (int i = 0; i < 5; ++i) {
        createArrayLeak();
    }
    
    // 暂时注释掉双重释放的代码，因为双重释放可能会导致程序崩溃
    // 制造2次双重释放：1次普通双重释放和1次数组双重释放
    // for (int i = 0; i < 1; ++i) {
    //     createDoubleFree();
    // }
    
    // for (int i = 0; i < 1; ++i) {
    //     createArrayDoubleFree();
    // }
    
    // 制造一些不会泄漏的内存分配和释放
    int* normal = new int;
    *normal = 100;
    delete normal;
    
    char* normal_array = new char[200];
    normal_array[0] = 'b';
    delete[] normal_array;
    
    std::cout << "Test completed. Checking for memory leaks..." << std::endl;
    
    return 0;
}
