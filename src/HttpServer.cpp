#include "HttpServer.h"
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

HttpServer::HttpServer(int port, Snowflake& snowflake) 
    : port_(port), snowflake_(snowflake) {
}

HttpServer::~HttpServer() {
    // 清理 Winsock
    WSACleanup();
}

void HttpServer::start() {
    WSADATA wsaData;
    int iResult;

    // 初始化 Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return;
    }

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // 解析服务器地址和端口
    std::string port_str = std::to_string(port_);
    iResult = getaddrinfo(NULL, port_str.c_str(), &hints, &result);
    if (iResult != 0) {
        std::cerr << "getaddrinfo failed: " << iResult << std::endl;
        WSACleanup();
        return;
    }

    SOCKET ListenSocket = INVALID_SOCKET;

    // 创建监听套接字
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return;
    }

    // 设置套接字选项，允许地址重用
    int optval = 1;
    iResult = setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "setsockopt failed: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        freeaddrinfo(result);
        WSACleanup();
        return;
    }

    // 绑定套接字到服务器地址和端口
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    freeaddrinfo(result);

    // 开始监听连接请求
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    std::cout << "HTTP server starting on port " << port_ << "...\n";
    std::cout << "Available endpoints:\n";
    std::cout << "  GET /api/nextid - Generate a new Snowflake ID\n";
    std::cout << "  GET / - Show API documentation\n";
    std::cout << "HTTP server is running on port " << port_ << "\n";

    // 接受和处理客户端连接
    while (true) {
        SOCKET ClientSocket = INVALID_SOCKET;

        // 接受客户端连接
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
            closesocket(ListenSocket);
            WSACleanup();
            return;
        }

        // 处理客户端请求
        handleClientRequest(ClientSocket);

        // 关闭客户端套接字
        closesocket(ClientSocket);
    }

    // 关闭监听套接字
    closesocket(ListenSocket);

    // 清理 Winsock
    WSACleanup();
}

void HttpServer::handleClientRequest(SOCKET client_socket) {
    char recvbuf[4096];
    int iResult;

    // 接收客户端请求
    iResult = recv(client_socket, recvbuf, sizeof(recvbuf), 0);
    if (iResult > 0) {
        recvbuf[iResult] = '\0';

        // 解析请求方法和路径
        std::string request(recvbuf);
        std::string method;
        std::string path;

        size_t space_pos = request.find(' ');
        if (space_pos != std::string::npos) {
            method = request.substr(0, space_pos);
            size_t next_space_pos = request.find(' ', space_pos + 1);
            if (next_space_pos != std::string::npos) {
                path = request.substr(space_pos + 1, next_space_pos - space_pos - 1);
            }
        }

        // 处理 GET 请求
        if (method == "GET") {
            if (path == "/api/nextid") {
                // 生成 Snowflake ID
                try {
                    int64_t id = snowflake_.nextId();
                    std::string json_response = "{\"id\":\"" + std::to_string(id) + "\"}";
                    sendResponse(client_socket, 200, "application/json", json_response);
                } catch (const std::exception& e) {
                    std::string error_response = "{\"error\":\"" + std::string(e.what()) + "\"}";
                    sendResponse(client_socket, 500, "application/json", error_response);
                }
            } else if (path == "/") {
                // 显示 API 文档
                std::string response = "Snowflake ID Generator API\n\nAvailable endpoints:\n- GET /api/nextid - Generate a new Snowflake ID";
                sendResponse(client_socket, 200, "text/plain", response);
            } else {
                // 处理 404 错误
                std::string response = "404 Not Found";
                sendResponse(client_socket, 404, "text/plain", response);
            }
        } else {
            // 处理不支持的请求方法
            std::string response = "405 Method Not Allowed";
            sendResponse(client_socket, 405, "text/plain", response);
        }
    } else if (iResult == 0) {
        std::cout << "Connection closing...\n";
    } else {
        std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
    }
}

void HttpServer::sendResponse(SOCKET client_socket, int status_code, const std::string& content_type, const std::string& response_body) {
    std::string status_message;
    switch (status_code) {
        case 200:
            status_message = "OK";
            break;
        case 404:
            status_message = "Not Found";
            break;
        case 405:
            status_message = "Method Not Allowed";
            break;
        case 500:
            status_message = "Internal Server Error";
            break;
        default:
            status_message = "Unknown Status";
            break;
    }

    // 构建响应头
    std::string response = "HTTP/1.1 " + std::to_string(status_code) + " " + status_message + "\r\n";
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + std::to_string(response_body.size()) + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";

    // 构建响应体
    response += response_body;

    // 发送响应
    int iResult = send(client_socket, response.c_str(), response.size(), 0);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "send failed: " << WSAGetLastError() << std::endl;
    }
}
