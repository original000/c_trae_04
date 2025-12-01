#pragma once

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <unordered_map>

class SimpleMemLeakDetector {
private:
    struct AllocationInfo {
        size_t size;
        
        AllocationInfo() : size(0) {}
        AllocationInfo(size_t s) : size(s) {}
    };

    std::unordered_map<void*, AllocationInfo> allocations;

    SimpleMemLeakDetector() {}
    ~SimpleMemLeakDetector() {
        GenerateLeakReport();
    }

    static SimpleMemLeakDetector& Instance() {
        static SimpleMemLeakDetector instance;
        return instance;
    }

    void* Allocate(size_t size) {
        void* ptr = malloc(size);
        if (ptr) {
            allocations[ptr] = AllocationInfo(size);
        }
        return ptr;
    }

    void Deallocate(void* ptr) {
        if (ptr) {
            allocations.erase(ptr);
        }
        free(ptr);
    }

    void GenerateLeakReport() {
        if (allocations.empty()) {
            return;
        }

        std::ofstream report("leak_report.txt");
        if (!report.is_open()) {
            return;
        }

        report << "Memory Leak Report" << std::endl;
        report << "==================" << std::endl;
        report << std::endl;

        size_t totalLeaked = 0;

        for (const auto& pair : allocations) {
            const AllocationInfo& info = pair.second;
            report << "Leak: " << info.size << " bytes at " << pair.first << std::endl;
            totalLeaked += info.size;
        }

        report << std::endl;
        report << "Total leaked memory: " << totalLeaked << " bytes" << std::endl;

        report.close();
    }

public:
    // 移除静态operator new和operator delete，避免与全局运算符重载冲突
};

// 全局运算符重载
void* operator new(size_t size) {
    return SimpleMemLeakDetector::Instance().Allocate(size);
}

void operator delete(void* ptr) noexcept {
    SimpleMemLeakDetector::Instance().Deallocate(ptr);
}

void operator delete(void* ptr, size_t size) noexcept {
    SimpleMemLeakDetector::Instance().Deallocate(ptr);
}

void* operator new[](size_t size) {
    return SimpleMemLeakDetector::Instance().Allocate(size);
}

void operator delete[](void* ptr) noexcept {
    SimpleMemLeakDetector::Instance().Deallocate(ptr);
}

void operator delete[](void* ptr, size_t size) noexcept {
    SimpleMemLeakDetector::Instance().Deallocate(ptr);
}