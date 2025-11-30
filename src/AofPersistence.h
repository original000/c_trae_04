#ifndef AOFPERSISTENCE_H
#define AOFPERSISTENCE_H

#include <string>
#include <fstream>
#include <thread>
#include <atomic>
#include <mutex>

class Database;

class AofPersistence {
public:
    AofPersistence(Database& db, const std::string& filename = "aof.appendonly");
    ~AofPersistence();

    // 启动AOF持久化（包括fsync线程）
    bool start();

    // 停止AOF持久化
    void stop();

    // 追加命令到AOF文件
    bool append_command(const std::string& command);

    // 从AOF文件加载数据到数据库
    bool load();

private:
    // fsync线程函数
    void fsync_thread_func();

    Database& db_;
    std::string filename_;
    std::ofstream file_;
    std::thread fsync_thread_;
    std::atomic<bool> running_;
    std::mutex mutex_;
};

#endif // AOFPERSISTENCE_H
