#ifndef SNOWFLAKE_H
#define SNOWFLAKE_H

#include <cstdint>
#include <mutex>

class Snowflake {
public:
    Snowflake(int64_t worker_id, int64_t datacenter_id);
    ~Snowflake() = default;

    int64_t nextId();

private:
    static const int64_t EPOCH = 1609459200000LL; // 2021-01-01 00:00:00 UTC
    static const int64_t WORKER_ID_BITS = 5LL;
    static const int64_t DATACENTER_ID_BITS = 5LL;
    static const int64_t MAX_WORKER_ID = (1LL << WORKER_ID_BITS) - 1LL;
    static const int64_t MAX_DATACENTER_ID = (1LL << DATACENTER_ID_BITS) - 1LL;
    static const int64_t SEQUENCE_BITS = 12LL;
    static const int64_t WORKER_ID_SHIFT = SEQUENCE_BITS;
    static const int64_t DATACENTER_ID_SHIFT = SEQUENCE_BITS + WORKER_ID_BITS;
    static const int64_t TIMESTAMP_LEFT_SHIFT = SEQUENCE_BITS + WORKER_ID_BITS + DATACENTER_ID_BITS;
    static const int64_t SEQUENCE_MASK = (1LL << SEQUENCE_BITS) - 1LL;

    int64_t worker_id_;
    int64_t datacenter_id_;
    int64_t last_timestamp_;
    int64_t sequence_;
    std::mutex mutex_;

    int64_t currentTimestamp();
    void sleepUntilNextMillis(int64_t last_timestamp);
};

#endif // SNOWFLAKE_H
