#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H

#include <cstddef>
#include <mutex>
#include <atomic>
#include <new> // 用于placement new
#include <cstdlib> // 用于malloc和free

// 通用对象池模板类
template <typename T, size_t Alignment = alignof(T)>
class ObjectPool {
public:
    // 构造函数
    explicit ObjectPool(size_t block_size = 1024)
        : block_size_(block_size),
          object_size_(aligned_size()),
          block_list_(nullptr),
          free_list_(nullptr),
          allocated_count_(0),
          peak_allocated_count_(0),
          peak_memory_(0) {
    }
    
    // 析构函数
    ~ObjectPool() {
        // 释放所有内存块
        MemoryBlock* current = block_list_;
        while (current != nullptr) {
            MemoryBlock* next = current->next;
            free(current);
            current = next;
        }
    }
    
    // 禁止拷贝和移动
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&&) = delete;
    ObjectPool& operator=(ObjectPool&&) = delete;
    
    // 分配对象
    template <typename... Args>
    T* allocate(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 如果空闲列表为空，分配新的内存块
        if (free_list_ == nullptr) {
            allocate_block();
        }
        
        // 从空闲列表中取出一个节点
        FreeNode* node = free_list_;
        free_list_ = node->next;
        
        // 使用placement new构造对象
        T* ptr = reinterpret_cast<T*>(node);
        new (ptr) T(std::forward<Args>(args)...);
        
        // 更新统计信息
        allocated_count_++;
        size_t current_allocated = allocated_count_.load();
        size_t current_peak = peak_allocated_count_.load();
        if (current_allocated > current_peak) {
            peak_allocated_count_.store(current_allocated);
        }
        
        return ptr;
    }
    
    // 释放对象
    void deallocate(T* ptr) {
        if (ptr == nullptr) {
            return;
        }
        
        // 调用对象的析构函数
        ptr->~T();
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 将对象添加到空闲列表
        FreeNode* node = reinterpret_cast<FreeNode*>(ptr);
        node->next = free_list_;
        free_list_ = node;
        
        // 更新统计信息
        allocated_count_--;
    }
    
    // 统计接口：当前分配的对象数量
    size_t allocated_count() const {
        return allocated_count_;
    }
    
    // 统计接口：峰值内存使用量
    size_t peak_memory() const {
        return peak_memory_;
    }
    
private:
    // 内存块结构体
    struct MemoryBlock {
        MemoryBlock* next;
        char data[1]; // 柔性数组，指向实际数据区域
    };
    
    // 空闲节点结构体
    struct FreeNode {
        FreeNode* next;
    };
    
    // 计算对齐后的大小
    static size_t aligned_size() {
        const size_t base_size = sizeof(T);
        const size_t alignment = Alignment;
        
        // 确保对齐值是2的幂
        static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be a power of 2");
        
        // 计算对齐后的大小
        return (base_size + alignment - 1) & ~(alignment - 1);
    }
    
    // 分配新的内存块
    void allocate_block() {
        // 计算内存块的大小
        const size_t block_memory_size = sizeof(MemoryBlock) + block_size_ * object_size_;
        
        // 分配内存块
        MemoryBlock* block = reinterpret_cast<MemoryBlock*>(malloc(block_memory_size));
        if (block == nullptr) {
            throw std::bad_alloc();
        }
        
        // 将内存块添加到块列表
        block->next = block_list_;
        block_list_ = block;
        
        // 将内存块中的所有对象添加到空闲列表
        char* data = block->data;
        for (size_t i = 0; i < block_size_; ++i) {
            FreeNode* node = reinterpret_cast<FreeNode*>(data + i * object_size_);
            node->next = free_list_;
            free_list_ = node;
        }
        
        // 更新峰值内存使用量
        size_t current_memory = 0;
        MemoryBlock* current = block_list_;
        while (current != nullptr) {
            current_memory += sizeof(MemoryBlock) + block_size_ * object_size_;
            current = current->next;
        }
        if (current_memory > peak_memory_) {
            peak_memory_ = current_memory;
        }
    }
    
    const size_t block_size_; // 每个内存块包含的对象数量
    const size_t object_size_; // 对齐后的对象大小
    
    MemoryBlock* block_list_; // 内存块列表
    FreeNode* free_list_; // 空闲节点列表
    
    std::mutex mutex_; // 互斥锁，保证线程安全
    
    std::atomic<size_t> allocated_count_; // 当前分配的对象数量
    std::atomic<size_t> peak_allocated_count_; // 峰值分配对象数量
    std::atomic<size_t> peak_memory_; // 峰值内存使用量
};

#endif // OBJECTPOOL_H