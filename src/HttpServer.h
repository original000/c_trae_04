#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <string>
#include <thread>
#include <atomic>

class Database;
class AofPersistence;

class HttpServer {
public:
    HttpServer(Database& db, AofPersistence& aof, int port = 8080);
    ~HttpServer();

    // 启动HTTP服务器
    bool start();

    // 停止HTTP服务器
    void stop();

private:
    // 服务器线程函数
    void server_thread_func();

    Database& db_;
    AofPersistence& aof_;
    int port_;
    std::thread server_thread_;
    std::atomic<bool> running_;
};

#endif // HTTPSERVER_H
