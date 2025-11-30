#include "HttpServer.h"
#include "httplib.h"
#include "../utils/Config.h"
#include "../utils/Logger.h"
#include "json.hpp"

using json = nlohmann::json;

HttpServer::HttpServer() : server_(new httplib::Server()) {
}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::init() {
    // Initialize the shorten service
    if (!shorten_service_.init()) {
        LOG_ERROR("Failed to initialize ShortenService");
        return false;
    }

    // Setup the HTTP routes
    setupRoutes();

    LOG_INFO("HttpServer initialized successfully");
    return true;
}

void HttpServer::start() {
    std::string address = Config::getInstance().getServerAddress();
    int port = Config::getInstance().getServerPort();

    LOG_INFO(std::string("Short Link Service running on http://") + address + ":" + std::to_string(port));
    LOG_INFO(std::string("访问 http://127.0.0.1:") + std::to_string(port) + "/s/abc123 能 302 跳转");

    // 启动服务器
    if (!server_->listen(address.c_str(), port)) {
        LOG_ERROR(std::string("Failed to start server on http://") + address + ":" + std::to_string(port));
        return;
    }

    // 保持服务器运行，直到收到停止信号
    while (true) {
        // 检查服务器是否仍在运行
        if (!server_->is_running()) {
            break;
        }

        // 等待一段时间再检查
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void HttpServer::stop() {
    // Stop the server
    if (server_) {
        server_->stop();
    }

    LOG_INFO("HttpServer stopped");
}

void HttpServer::setupRoutes() {
    // POST /api/v1/shorten - 创建短链接
    server_->Post("/api/v1/shorten", [this](const httplib::Request& req, httplib::Response& res) {
        handleCreateShortLink(req, res);
    });

    // GET /s/{code} - 302 重定向
    server_->Get(R"(/s/([a-zA-Z0-9]+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleRedirect(req, res);
    });

    // GET /api/v1/links/{id}/stats - 返回详情 + 最近 20 次访问记录
    server_->Get(R"(/api/v1/links/([0-9]+)/stats)", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetLinkStats(req, res);
    });

    // POST /api/v1/links/{id}/disable - 禁用
    server_->Post(R"(/api/v1/links/([0-9]+)/disable)", [this](const httplib::Request& req, httplib::Response& res) {
        handleDisableLink(req, res);
    });
}

void HttpServer::handleCreateShortLink(const httplib::Request& req, httplib::Response& res) {
    try {
        // Parse the request body as JSON
        json request_body = json::parse(req.body);

        if (!request_body.contains("long_url")) {
            json error_response;
            error_response["error"] = "Missing required parameter 'long_url'";
            res.status = 400;
            res.set_content(error_response.dump(), "application/json");
            return;
        }

        std::string long_url = request_body["long_url"].dump();
        // Remove quotes from the dumped string
        if (long_url.size() >= 2 && long_url.front() == '"' && long_url.back() == '"') {
            long_url = long_url.substr(1, long_url.size() - 2);
        }
        if (long_url.empty()) {
            json error_response;
            error_response["error"] = "Parameter 'long_url' cannot be empty";
            res.status = 400;
            res.set_content(error_response.dump(), "application/json");
            return;
        }

        // Get optional parameters
        std::string custom_alias = "";
        if (request_body.contains("custom_alias")) {
            custom_alias = request_body["custom_alias"].dump();
            // Remove quotes from the dumped string
            if (custom_alias.size() >= 2 && custom_alias.front() == '"' && custom_alias.back() == '"') {
                custom_alias = custom_alias.substr(1, custom_alias.size() - 2);
            }
        }

        int expire_seconds = 0;
        if (request_body.contains("expire_seconds")) {
            expire_seconds = request_body["expire_seconds"].get<int>();
        }

        // Create the short link
        int id = shorten_service_.createShortLink(long_url, custom_alias, expire_seconds);

        // Get the created link to return the short code
        ShortLink link = shorten_service_.getStorage().getShortLinkById(id);

        // Prepare the response
        json response_body;
        response_body["id"] = id;
        response_body["short_code"] = link.short_code;
        response_body["long_url"] = link.long_url;
        response_body["custom_alias"] = link.custom_alias;
        response_body["created_at"] = shorten_service_.getStorage().timePointToString(link.created_at);
        response_body["expire_at"] = shorten_service_.getStorage().timePointToString(link.expire_at);
        response_body["disabled"] = link.disabled;

        res.status = 201;
        res.set_content(response_body.dump(), "application/json");

        LOG_INFO(std::string("Created short link for URL: ") + long_url);
    } catch (json::parse_error& e) {
        json error_response;
        error_response["error"] = "Invalid JSON format in request body";
        res.status = 400;
        res.set_content(error_response.dump(), "application/json");
        LOG_ERROR(std::string("JSON parse error: ") + e.what());
    } catch (std::exception& e) {
        json error_response;
        error_response["error"] = e.what();
        res.status = 500;
        res.set_content(error_response.dump(), "application/json");
        LOG_ERROR(std::string("Failed to create short link: ") + e.what());
    }
}

void HttpServer::handleRedirect(const httplib::Request& req, httplib::Response& res) {
    try {
        // Get the short code from the URL path
        std::string short_code = req.matches[1];
        if (short_code.empty()) {
            json error_response;
            error_response["error"] = "Missing short code in URL";
            res.status = 400;
            res.set_content(error_response.dump(), "application/json");
            return;
        }

        // Get the client IP and user agent
        std::string ip = getRequestIP(req);
        std::string user_agent = getRequestUA(req);

        // Get the long URL for the short code
        std::string long_url = shorten_service_.getLongUrl(short_code, ip, user_agent);

        // Redirect to the long URL
        res.status = 302;
        res.set_header("Location", long_url);

        LOG_INFO(std::string("Redirected short code: ") + short_code + " -> " + long_url);
    } catch (const std::exception& e) {
        json error_response;
        error_response["error"] = e.what();
        res.status = 404;
        res.set_content(error_response.dump(), "application/json");
        LOG_ERROR(std::string("Failed to redirect short code: ") + e.what());
    }
}

void HttpServer::handleGetLinkStats(const httplib::Request& req, httplib::Response& res) {
    try {
        // Get the link ID from the URL path
        int id = std::stoi(req.matches[1]);

        // Get the link stats
        LinkStats stats = shorten_service_.getLinkStats(id);

        // Prepare the response
        json response_body = {
            {"id", stats.id},
            {"short_code", stats.short_code},
            {"long_url", stats.long_url},
            {"custom_alias", stats.custom_alias},
            {"created_at", shorten_service_.getStorage().timePointToString(stats.created_at)},
            {"expire_at", shorten_service_.getStorage().timePointToString(stats.expire_at)},
            {"disabled", stats.disabled},
            {"total_accesses", stats.total_accesses}
        };

        // Add recent access logs
        json recent_accesses = json::array();
        for (const auto& log : stats.recent_accesses) {
            recent_accesses.push_back({
                {"ip", log.ip},
                {"user_agent", log.user_agent},
                {"accessed_at", shorten_service_.getStorage().timePointToString(log.accessed_at)}
            });
        }
        response_body["recent_accesses"] = recent_accesses;

        res.status = 200;
        res.set_content(response_body.dump(), "application/json");

        LOG_INFO(std::string("Retrieved stats for link ID: ") + std::to_string(id));
    } catch (const std::invalid_argument& e) {
        json error_response;
        error_response["error"] = "Invalid link ID format";
        res.status = 400;
        res.set_content(error_response.dump(), "application/json");
        LOG_ERROR(std::string("Invalid link ID: ") + e.what());
    } catch (const std::exception& e) {
        json error_response;
        error_response["error"] = e.what();
        res.status = 500;
        res.set_content(error_response.dump(), "application/json");
        LOG_ERROR(std::string("Failed to get link stats: ") + e.what());
    }
}

void HttpServer::handleDisableLink(const httplib::Request& req, httplib::Response& res) {
    int id = 0;
    try {
        // Get the link ID from the URL path
        id = std::stoi(req.matches[1]);

        // Disable the link
        bool success = shorten_service_.disableShortLink(id);

        if (success) {
            json success_response;
            success_response["message"] = "Short link disabled successfully";
            res.status = 200;
            res.set_content(success_response.dump(), "application/json");
            LOG_INFO(std::string("Disabled link ID: ") + std::to_string(id));
        } else {
            json error_response;
            error_response["error"] = "Short link not found";
            res.status = 404;
            res.set_content(error_response.dump(), "application/json");
            LOG_WARNING(std::string("Failed to disable link ID: ") + std::to_string(id) + " (not found)");
        }
    } catch (const std::invalid_argument& e) {
        json error_response;
        error_response["error"] = "Invalid link ID format";
        res.status = 400;
        res.set_content(error_response.dump(), "application/json");
        LOG_ERROR(std::string("Invalid link ID: ") + e.what());
    } catch (const std::exception& e) {
        json error_response;
        error_response["error"] = e.what();
        res.status = 500;
        res.set_content(error_response.dump(), "application/json");
        LOG_ERROR(std::string("Failed to disable link ID: ") + std::to_string(id) + ": " + e.what());
    }
}

std::string HttpServer::getRequestIP(const httplib::Request& req) {
    // Check for X-Forwarded-For header (common in reverse proxies)
    auto it = req.headers.find("X-Forwarded-For");
    if (it != req.headers.end()) {
        // X-Forwarded-For can contain a comma-separated list of IPs
        // The first one is the original client IP
        size_t comma_pos = it->second.find(',');
        if (comma_pos != std::string::npos) {
            return it->second.substr(0, comma_pos);
        }
        return it->second;
    }

    // Check for X-Real-IP header (also used by some proxies)
    it = req.headers.find("X-Real-IP");
    if (it != req.headers.end()) {
        return it->second;
    }

    // If no proxy headers, return the remote address from the request
    return req.remote_addr;
}

std::string HttpServer::getRequestUA(const httplib::Request& req) {
    auto it = req.headers.find("User-Agent");
    if (it != req.headers.end()) {
        return it->second;
    }
    return "Unknown";
}
