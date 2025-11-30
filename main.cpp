#include "ObjectPool.h"
#include <iostream>
#include <vector>
#include <chrono>

// 测试用的大对象结构体
struct Big {
    double d[100];
    Big(int x) {}
};

// 计算函数执行时间的辅助函数
template <typename Func>
double measure_time(Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    return duration.count();
}

int main() {
    const size_t object_count = 10000; // 增加对象数量到10000
    
    std::cout << "=== Object Pool Performance Test ===\n";
    
    // 测试1：使用对象池分配/释放Big对象
    ObjectPool<Big> pool(object_count); // 每个块包含100个对象
    
    double pool_time = measure_time([&]() {
        std::vector<Big*> objects;
        objects.reserve(object_count);
        
        // 分配100个对象
        for (size_t i = 0; i < object_count; ++i) {
            objects.push_back(pool.allocate(i));
        }
        
        // 释放100个对象
        for (Big* obj : objects) {
            pool.deallocate(obj);
        }
    });
    
    std::cout << "Time using object pool: " << pool_time << " ms\n";
    std::cout << "Object pool peak memory: " << pool.peak_memory() << " bytes\n";
    
    // 测试2：使用new/delete分配/释放Big对象
    double new_delete_time = measure_time([&]() {
        std::vector<Big*> objects;
        objects.reserve(object_count);
        
        // 分配100个对象
        for (size_t i = 0; i < object_count; ++i) {
            objects.push_back(new Big(i));
        }
        
        // 释放100个对象
        for (Big* obj : objects) {
            delete obj;
        }
    });
    
    std::cout << "Time using new/delete: " << new_delete_time << " ms\n";
    
    // Calculate performance improvement ratio
    double improvement = ((new_delete_time - pool_time) / new_delete_time) * 100;
    std::cout << "Performance improvement: " << improvement << "%\n";
    
    // Test 3: Allocate/Deallocate different types of objects using object pool
    std::cout << "\n=== Different Types of Objects Test ===\n";
    
    ObjectPool<int> int_pool;
    int* int_ptr = int_pool.allocate(42);
    std::cout << "Allocate int object: " << *int_ptr << "\n";
    int_pool.deallocate(int_ptr);
    
    ObjectPool<double> double_pool;
    double* double_ptr = double_pool.allocate(3.14159);
    std::cout << "Allocate double object: " << *double_ptr << "\n";
    double_pool.deallocate(double_ptr);
    
    std::cout << "\n=== Test Completed ===\n";
    
    return 0;
}
