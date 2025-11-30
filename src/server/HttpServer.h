#pragma once

#include <string>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "../service/ShortenService.h"

// Forward declarations of httplib classes
namespace httplib {
    class Server;
    class Request;
    class Response;
}

class HttpServer {
public:
    HttpServer();
    ~HttpServer();

    bool init();
    void start();
    void stop();

private:
    void setupRoutes();
    void threadPoolWorker();

    void handleCreateShortLink(const httplib::Request& req, httplib::Response& res);
    void handleRedirect(const httplib::Request& req, httplib::Response& res);
    void handleGetLinkStats(const httplib::Request& req, httplib::Response& res);
    void handleDisableLink(const httplib::Request& req, httplib::Response& res);

    std::string getRequestIP(const httplib::Request& req);
    std::string getRequestUA(const httplib::Request& req);

    std::unique_ptr<httplib::Server> server_;
    ShortenService shorten_service_;

    // Thread pool
    int thread_pool_size_;
    std::vector<std::thread> worker_threads_;
    std::queue<std::function<void()>> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cond_var_;
    bool stop_threads_;
};
