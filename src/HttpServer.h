#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <string>
#include "Snowflake.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class HttpServer {
public:
    HttpServer(int port, Snowflake& snowflake);
    ~HttpServer();
    void start();

private:
    void handleClientRequest(SOCKET client_socket);
    void sendResponse(SOCKET client_socket, int status_code, const std::string& content_type, const std::string& response_body);

    int port_;
    Snowflake& snowflake_;
};

#endif // HTTPSERVER_H
