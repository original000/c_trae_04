#pragma once
// #include "StackTraceWin.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

class MemLeakDetector {
public:
    static MemLeakDetector& Instance() {
        static MemLeakDetector instance;
        return instance;
    }
    
    void* Allocate(size_t size, bool is_array) {
        void* ptr = ::operator new(size);
        allocations_[ptr] = AllocationInfo(size, is_array);
        return ptr;
    }
    
    void Deallocate(void* ptr, bool is_array) {
        auto it = allocations_.find(ptr);
        if (it == allocations_.end()) {
            // 检测到双重释放或释放未分配的内存
            std::cerr << "ERROR: Double free or free of unallocated memory at address 0x" 
                      << std::hex << (uintptr_t)ptr << std::dec << std::endl;
            return;
        }
        
        if (it->second.is_array != is_array) {
            // 检测到不匹配的delete/delete[]
            std::cerr << "ERROR: Mismatched delete/delete[] for address 0x" 
                      << std::hex << (uintptr_t)ptr << std::dec << std::endl;
        }
        
        if (is_array) {
            ::operator delete[](ptr);
        } else {
            ::operator delete(ptr);
        }
        allocations_.erase(it);
    }
    
    void AddToIgnoreList(const std::string& function_name) {
        ignore_list_.insert(function_name);
    }
    
    void GenerateLeakReport() {
        if (allocations_.empty()) {
            return;
        }
        
        // 计算总泄漏内存
        size_t total_leaked = 0;
        for (const auto& allocation : allocations_) {
            total_leaked += allocation.second.size;
        }
        
        // 生成泄漏报告文件
        std::ofstream report("leak_report.txt");
        if (report.is_open()) {
            report << "Memory Leak Report" << std::endl;
            report << "==================" << std::endl;
            report << "Total leaked memory: " << total_leaked << " bytes" << std::endl;
            report << std::endl;
            
            for (const auto& pair : allocations_) {
                const AllocationInfo& info = pair.second;
                report << "Leak: " << info.size << " bytes at " << pair.first;
                report << (info.is_array ? " (array)" : "") << std::endl;
            }
            
            report.close();
        } else {
            std::cerr << "ERROR: Failed to create leak_report.txt" << std::endl;
        }
    }
    
private:
    struct AllocationInfo {
        size_t size;
        bool is_array;
        
        // 默认构造函数
        AllocationInfo() : size(0), is_array(false) {}
        
        AllocationInfo(size_t s, bool arr) 
            : size(s), is_array(arr) {}
    };
    
    std::unordered_map<void*, AllocationInfo> allocations_;
    std::unordered_set<std::string> ignore_list_;
    
    MemLeakDetector() {}
    ~MemLeakDetector() {
        GenerateLeakReport();
    }
    
    MemLeakDetector(const MemLeakDetector&) = delete;
    MemLeakDetector& operator=(const MemLeakDetector&) = delete;
};

// 重载全局new/delete/new[]/delete[]
inline void* operator new(size_t size) {
    return MemLeakDetector::Instance().Allocate(size, false);
}

inline void operator delete(void* ptr) noexcept {
    MemLeakDetector::Instance().Deallocate(ptr, false);
}

// 带大小参数的operator delete
inline void operator delete(void* ptr, size_t /*size*/) noexcept {
    MemLeakDetector::Instance().Deallocate(ptr, false);
}

inline void* operator new[](size_t size) {
    return MemLeakDetector::Instance().Allocate(size, true);
}

inline void operator delete[](void* ptr) noexcept {
    MemLeakDetector::Instance().Deallocate(ptr, true);
}

// 带大小参数的operator delete[]
inline void operator delete[](void* ptr, size_t /*size*/) noexcept {
    MemLeakDetector::Instance().Deallocate(ptr, true);
}

// 为了兼容C++标准，还需要重载这些版本
inline void* operator new(size_t size, const std::nothrow_t&) noexcept {
    try {
        return operator new(size);
    } catch (...) {
        return nullptr;
    }
}

inline void operator delete(void* ptr, const std::nothrow_t&) noexcept {
    operator delete(ptr);
}

inline void* operator new[](size_t size, const std::nothrow_t&) noexcept {
    try {
        return operator new[](size);
    } catch (...) {
        return nullptr;
    }
}

inline void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
    operator delete[](ptr);
}
