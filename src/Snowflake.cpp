#include "Snowflake.h"
#include <chrono>
#include <thread>
#include <stdexcept>

Snowflake::Snowflake(int64_t worker_id, int64_t datacenter_id) {
    if (worker_id < 0 || worker_id > MAX_WORKER_ID) {
        throw std::invalid_argument("worker_id must be between 0 and " + std::to_string(MAX_WORKER_ID));
    }
    if (datacenter_id < 0 || datacenter_id > MAX_DATACENTER_ID) {
        throw std::invalid_argument("datacenter_id must be between 0 and " + std::to_string(MAX_DATACENTER_ID));
    }

    worker_id_ = worker_id;
    datacenter_id_ = datacenter_id;
    last_timestamp_ = -1LL;
    sequence_ = 0LL;
}

int64_t Snowflake::nextId() {
    std::lock_guard<std::mutex> lock(mutex_);

    int64_t timestamp = currentTimestamp();

    // 处理时钟回拨
    if (timestamp < last_timestamp_) {
        // 计算需要等待的时间
        int64_t wait_time = last_timestamp_ - timestamp;
        sleepUntilNextMillis(last_timestamp_);
        timestamp = currentTimestamp();
        
        // 再次检查是否追上了时钟
        if (timestamp < last_timestamp_) {
            throw std::runtime_error("Clock moved backwards too much. Cannot generate ID.");
        }
    }

    // 如果是同一毫秒，递增序列号
    if (timestamp == last_timestamp_) {
        sequence_ = (sequence_ + 1LL) & SEQUENCE_MASK;
        
        // 如果序列号溢出，等待到下一毫秒
        if (sequence_ == 0LL) {
            sleepUntilNextMillis(last_timestamp_);
            timestamp = currentTimestamp();
        }
    } else {
        // 不同毫秒，重置序列号
        sequence_ = 0LL;
    }

    last_timestamp_ = timestamp;

    // 组合ID：时间戳 << 22 | 数据中心ID << 17 | 工作节点ID << 12 | 序列号
    return ((timestamp - EPOCH) << TIMESTAMP_LEFT_SHIFT) |
           (datacenter_id_ << DATACENTER_ID_SHIFT) |
           (worker_id_ << WORKER_ID_SHIFT) |
           sequence_;
}

int64_t Snowflake::currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

void Snowflake::sleepUntilNextMillis(int64_t last_timestamp) {
    int64_t timestamp = currentTimestamp();
    while (timestamp <= last_timestamp) {
        timestamp = currentTimestamp();
        // 短暂睡眠，避免CPU占用过高
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}
