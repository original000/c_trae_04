#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <set>
#include "Snowflake.h"

// 测试递增性
TEST(SnowflakeTest, Incremental) {
    Snowflake snowflake(1, 1);
    int64_t id1 = snowflake.nextId();
    int64_t id2 = snowflake.nextId();
    int64_t id3 = snowflake.nextId();
    
    EXPECT_LT(id1, id2);
    EXPECT_LT(id2, id3);
}

// 测试多线程唯一性
TEST(SnowflakeTest, ThreadSafe) {
    Snowflake snowflake(1, 1);
    std::set<int64_t> ids;
    std::vector<std::thread> threads;
    const int thread_count = 10;
    const int ids_per_thread = 1000;
    
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < ids_per_thread; ++j) {
                int64_t id = snowflake.nextId();
                std::lock_guard<std::mutex> lock(mutex);
                ids.insert(id);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(ids.size(), thread_count * ids_per_thread);
}

// 测试时钟回拨处理
TEST(SnowflakeTest, ClockBackward) {
    Snowflake snowflake(1, 1);
    int64_t id1 = snowflake.nextId();
    
    // 模拟时钟回拨：将系统时间设置为1秒前
    // 注意：这只是一个模拟，实际测试中可能需要更复杂的设置
    // 这里我们通过创建一个新的Snowflake实例并手动设置last_timestamp_来模拟
    
    // 由于Snowflake类的last_timestamp_是私有成员，我们无法直接访问
    // 因此，我们将通过创建一个继承自Snowflake的测试类来访问私有成员
    class TestSnowflake : public Snowflake {
    public:
        TestSnowflake(int64_t worker_id, int64_t datacenter_id) 
            : Snowflake(worker_id, datacenter_id) {
        }
        
        void setLastTimestamp(int64_t timestamp) {
            last_timestamp_ = timestamp;
        }
        
        int64_t getLastTimestamp() const {
            return last_timestamp_;
        }
    };
    
    TestSnowflake test_snowflake(1, 1);
    int64_t id2 = test_snowflake.nextId();
    
    // 将last_timestamp_设置为比当前时间大1秒的值，模拟时钟回拨
    test_snowflake.setLastTimestamp(test_snowflake.getLastTimestamp() + 1000);
    
    // 再次生成ID，应该会等待到时钟追上
    int64_t id3 = test_snowflake.nextId();
    
    EXPECT_LT(id2, id3);
}

// 测试workerId边界值
TEST(SnowflakeTest, WorkerIdBoundary) {
    // 测试最小workerId
    Snowflake snowflake_min(0, 1);
    int64_t id_min = snowflake_min.nextId();
    EXPECT_GT(id_min, 0);
    
    // 测试最大workerId
    Snowflake snowflake_max(31, 1);
    int64_t id_max = snowflake_max.nextId();
    EXPECT_GT(id_max, 0);
    
    // 测试超过最大workerId的值，应该抛出异常
    EXPECT_THROW(Snowflake snowflake_invalid(32, 1), std::invalid_argument);
}

// 测试datacenterId边界值
TEST(SnowflakeTest, DatacenterIdBoundary) {
    // 测试最小datacenterId
    Snowflake snowflake_min(1, 0);
    int64_t id_min = snowflake_min.nextId();
    EXPECT_GT(id_min, 0);
    
    // 测试最大datacenterId
    Snowflake snowflake_max(1, 31);
    int64_t id_max = snowflake_max.nextId();
    EXPECT_GT(id_max, 0);
    
    // 测试超过最大datacenterId的值，应该抛出异常
    EXPECT_THROW(Snowflake snowflake_invalid(1, 32), std::invalid_argument);
}

// 测试序列号溢出处理
TEST(SnowflakeTest, SequenceOverflow) {
    Snowflake snowflake(1, 1);
    
    // 生成4096个ID，应该会触发序列号溢出
    for (int i = 0; i < 4096; ++i) {
        int64_t id = snowflake.nextId();
        EXPECT_GT(id, 0);
    }
    
    // 再次生成ID，应该会使用新的时间戳
    int64_t id = snowflake.nextId();
    EXPECT_GT(id, 0);
}

// 测试ID结构是否正确
TEST(SnowflakeTest, IdStructure) {
    Snowflake snowflake(1, 1);
    int64_t id = snowflake.nextId();
    
    // 提取各个部分
    int64_t timestamp = (id >> 22) + Snowflake::EPOCH;
    int64_t datacenter_id = (id >> 17) & 0x1F;
    int64_t worker_id = (id >> 12) & 0x1F;
    int64_t sequence = id & 0xFFF;
    
    // 验证各个部分是否正确
    EXPECT_EQ(datacenter_id, 1);
    EXPECT_EQ(worker_id, 1);
    EXPECT_GE(sequence, 0);
    EXPECT_LE(sequence, 4095);
    
    // 验证时间戳是否合理（不早于当前时间1秒）
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    EXPECT_GE(timestamp, now_ms - 1000);
    EXPECT_LE(timestamp, now_ms + 1000);
}

// 测试多个实例的ID唯一性
TEST(SnowflakeTest, MultipleInstances) {
    Snowflake snowflake1(1, 1);
    Snowflake snowflake2(2, 1);
    Snowflake snowflake3(1, 2);
    Snowflake snowflake4(2, 2);
    
    std::set<int64_t> ids;
    ids.insert(snowflake1.nextId());
    ids.insert(snowflake2.nextId());
    ids.insert(snowflake3.nextId());
    ids.insert(snowflake4.nextId());
    
    EXPECT_EQ(ids.size(), 4);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
