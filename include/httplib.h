// httplib.h - A C++11 header-only HTTP library
// https://github.com/yhirose/cpp-httplib

#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace httplib {

constexpr int MAX_RECV_BUF_SIZE = 8192;
constexpr int MAX_SEND_BUF_SIZE = 8192;

#ifdef _WIN32
using socket_t = SOCKET;
constexpr socket_t INVALID_SOCKET_ = INVALID_SOCKET;
#else
using socket_t = int;
constexpr socket_t INVALID_SOCKET_ = -1;
#endif

struct Request {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> get_params;
    std::map<std::string, std::string> post_params;
};

struct Response {
    std::string version = "HTTP/1.1";
    int status = 200;
    std::string reason = "OK";
    std::map<std::string, std::string> headers;
    std::string body;

    void set_header(const std::string& key, const std::string& value) {
        headers[key] = value;
    }

    void set_content(const std::string& content, const std::string& content_type) {
        body = content;
        set_header("Content-Type", content_type);
        char buf[32];
        snprintf(buf, sizeof(buf), "%zu", body.size());
        set_header("Content-Length", buf);
    }
};

using Handler = std::function<void(const Request&, Response&)>;
using Method = std::string;

class Server {
public:
    Server() : socket_(INVALID_SOCKET_) {
#ifdef _WIN32
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    }

    ~Server() {
        stop();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    bool is_running() const {
        return socket_ != INVALID_SOCKET_;
    }

    bool listen(const std::string& host, int port) {
        socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (socket_ == INVALID_SOCKET_) {
            return false;
        }

        int opt = 1;
#ifdef _WIN32
        setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
        setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(host.c_str());

        if (::bind(socket_, (sockaddr*)&addr, sizeof(addr)) == -1) {
            close_socket(socket_);
            socket_ = INVALID_SOCKET_;
            return false;
        }

        if (::listen(socket_, SOMAXCONN) == -1) {
            close_socket(socket_);
            socket_ = INVALID_SOCKET_;
            return false;
        }

        return true;
    }

    void stop() {
        if (socket_ != INVALID_SOCKET_) {
            close_socket(socket_);
            socket_ = INVALID_SOCKET_;
        }
    }

    void handle(const Method& method, const std::string& pattern, const Handler& handler) {
        routes_.emplace_back(method, pattern, handler);
    }

    void get(const std::string& pattern, const Handler& handler) {
        handle("GET", pattern, handler);
    }

    void post(const std::string& pattern, const Handler& handler) {
        handle("POST", pattern, handler);
    }

    void put(const std::string& pattern, const Handler& handler) {
        handle("PUT", pattern, handler);
    }

    void delete_(const std::string& pattern, const Handler& handler) {
        handle("DELETE", pattern, handler);
    }

    void options(const std::string& pattern, const Handler& handler) {
        handle("OPTIONS", pattern, handler);
    }

    void patch(const std::string& pattern, const Handler& handler) {
        handle("PATCH", pattern, handler);
    }

    void head(const std::string& pattern, const Handler& handler) {
        handle("HEAD", pattern, handler);
    }

    void run() {
        while (is_running()) {
            sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);

            socket_t client_socket = ::accept(socket_, (sockaddr*)&client_addr, &client_addr_len);
            if (client_socket == INVALID_SOCKET_) {
                continue;
            }

            handle_client(client_socket);
        }
    }

private:
    struct Route {
        Method method;
        std::string pattern;
        Handler handler;
        std::regex regex;

        Route(const Method& method, const std::string& pattern, const Handler& handler)
            : method(method), pattern(pattern), handler(handler), regex(build_regex(pattern)) {
        }

        static std::regex build_regex(const std::string& pattern) {
            std::string regex_pattern = "^";
            size_t pos = 0;

            while (pos < pattern.size()) {
                if (pattern[pos] == ':') {
                    size_t end = pattern.find('/', pos);
                    if (end == std::string::npos) {
                        end = pattern.size();
                    }
                    regex_pattern += "([^/]+)";
                    pos = end;
                } else {
                    regex_pattern += pattern[pos];
                    pos++;
                }
            }

            regex_pattern += "$";
            return std::regex(regex_pattern);
        }
    };

    void handle_client(socket_t client_socket) {
        char buf[MAX_RECV_BUF_SIZE];
        std::string request_data;

        while (true) {
            int n = recv(client_socket, buf, sizeof(buf), 0);
            if (n <= 0) {
                break;
            }

            request_data.append(buf, n);

            // 检查是否收到完整的请求
            if (request_data.find("\r\n\r\n") != std::string::npos) {
                break;
            }
        }

        if (request_data.empty()) {
            close_socket(client_socket);
            return;
        }

        Request req;
        parse_request(request_data, req);

        Response res;
        handle_request(req, res);

        send_response(client_socket, res);

        close_socket(client_socket);
    }

    void parse_request(const std::string& data, Request& req) {
        std::istringstream iss(data);
        std::string line;

        // 解析请求行
        if (std::getline(iss, line)) {
            std::istringstream line_iss(line);
            line_iss >> req.method >> req.path >> req.version;
        }

        // 解析请求头
        while (std::getline(iss, line) && line != "\r") {
            if (line.back() == '\r') {
                line.pop_back();
            }

            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);

                // 去除值前后的空格
                size_t start = value.find_first_not_of(' ');
                size_t end = value.find_last_not_of(' ');
                if (start != std::string::npos && end != std::string::npos) {
                    value = value.substr(start, end - start + 1);
                }

                req.headers[key] = value;
            }
        }

        // 解析请求体
        if (req.headers.count("Content-Length")) {
            int content_length = std::stoi(req.headers["Content-Length"]);
            req.body = data.substr(data.find("\r\n\r\n") + 4, content_length);
        }

        // 解析查询字符串
        size_t query_pos = req.path.find('?');
        if (query_pos != std::string::npos) {
            std::string query_string = req.path.substr(query_pos + 1);
            req.path = req.path.substr(0, query_pos);
            parse_query_string(query_string, req.get_params);
        }

        // 解析POST参数
        if (req.method == "POST" && req.headers.count("Content-Type")) {
            std::string content_type = req.headers["Content-Type"];
            if (content_type.find("application/x-www-form-urlencoded") != std::string::npos) {
                parse_query_string(req.body, req.post_params);
            }
        }
    }

    void parse_query_string(const std::string& query_string, std::map<std::string, std::string>& params) {
        std::istringstream iss(query_string);
        std::string pair;

        while (std::getline(iss, pair, '&')) {
            size_t equal_pos = pair.find('=');
            if (equal_pos != std::string::npos) {
                std::string key = url_decode(pair.substr(0, equal_pos));
                std::string value = url_decode(pair.substr(equal_pos + 1));
                params[key] = value;
            } else {
                std::string key = url_decode(pair);
                params[key] = "";
            }
        }
    }

    std::string url_decode(const std::string& str) {
        std::string result;
        result.reserve(str.size());

        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '%') {
                if (i + 2 < str.size()) {
                    std::string hex = str.substr(i + 1, 2);
                    char c = static_cast<char>(std::stoi(hex, nullptr, 16));
                    result += c;
                    i += 2;
                }
            } else if (str[i] == '+') {
                result += ' ';
            } else {
                result += str[i];
            }
        }

        return result;
    }

    void handle_request(const Request& req, Response& res) {
        for (const auto& route : routes_) {
            if (route.method == req.method) {
                std::smatch match;
                if (std::regex_match(req.path, match, route.regex)) {
                    // 提取路径参数
                    Request req_with_params = req;
                    size_t param_index = 1;

                    for (size_t i = 0; i < route.pattern.size(); ++i) {
                        if (route.pattern[i] == ':') {
                            size_t end = route.pattern.find('/', i);
                            if (end == std::string::npos) {
                                end = route.pattern.size();
                            }

                            std::string param_name = route.pattern.substr(i + 1, end - i - 1);
                            req_with_params.params[param_name] = match[param_index].str();
                            param_index++;

                            i = end;
                        }
                    }

                    route.handler(req_with_params, res);
                    return;
                }
            }
        }

        // 如果没有找到匹配的路由，返回404
        res.status = 404;
        res.reason = "Not Found";
        res.set_content("404 Not Found", "text/plain");
    }

    void send_response(socket_t client_socket, const Response& res) {
        std::ostringstream oss;

        // 写入状态行
        oss << res.version << " " << res.status << " " << res.reason << "\r\n";

        // 写入响应头
        for (const auto& header : res.headers) {
            oss << header.first << ": " << header.second << "\r\n";
        }

        // 写入空行分隔响应头和响应体
        oss << "\r\n";

        // 写入响应体
        oss << res.body;

        std::string response_data = oss.str();
        const char* data_ptr = response_data.c_str();
        size_t data_size = response_data.size();
        size_t sent_size = 0;

        while (sent_size < data_size) {
            int n = send(client_socket, data_ptr + sent_size, data_size - sent_size, 0);
            if (n <= 0) {
                break;
            }

            sent_size += n;
        }
    }

    void close_socket(socket_t socket) {
#ifdef _WIN32
        closesocket(socket);
#else
        close(socket);
#endif
    }

    socket_t socket_;
    std::vector<Route> routes_;
};

} // namespace httplib
