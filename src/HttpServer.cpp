#include "HttpServer.h"
#include "Database.h"
#include "AofPersistence.h"
#include "Command.h"
#include "httplib.h"
#include <iostream>
#include <sstream>

HttpServer::HttpServer(Database& db, AofPersistence& aof, int port)
    : db_(db), aof_(aof), port_(port), running_(false) {
}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start() {
    running_ = true;
    server_thread_ = std::thread(&HttpServer::server_thread_func, this);
    return true;
}

void HttpServer::stop() {
    if (running_) {
        running_ = false;
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }
}

void HttpServer::server_thread_func() {
    httplib::Server server;

    // 处理/cmd接口
    server.get("/cmd", [this](const httplib::Request& req, httplib::Response& res) {
        CommandParser parser;

        // 解析查询字符串中的cmd参数
        ParsedCommand* parsed_cmd = parser.parse_http_query(req.path);
        if (!parsed_cmd) {
            res.set_content("Invalid command", "text/plain");
            res.status = 400;
            return;
        }

        // 执行命令
        std::string response;
        bool success = false;

        switch (parsed_cmd->type) {
            case CommandType::SET: {
                if (parsed_cmd->args.size() >= 2) {
                    int expire_seconds = 0;
                    std::string cmd_str = "SET " + parsed_cmd->args[0] + " " + parsed_cmd->args[1];

                    for (size_t i = 2; i < parsed_cmd->args.size(); ++i) {
                        if (parsed_cmd->args[i] == "EX" && i + 1 < parsed_cmd->args.size()) {
                            expire_seconds = std::stoi(parsed_cmd->args[i + 1]);
                            cmd_str += " EX " + parsed_cmd->args[i + 1];
                            break;
                        }
                    }

                    success = db_.set(parsed_cmd->args[0], parsed_cmd->args[1], expire_seconds);
                    if (success) {
                        aof_.append_command(cmd_str);
                        response = "OK";
                    } else {
                        response = "ERROR";
                    }
                } else {
                    response = "Invalid number of arguments for SET command";
                }
                break;
            }
            case CommandType::GET: {
                if (parsed_cmd->args.size() >= 1) {
                    std::string* value = db_.get(parsed_cmd->args[0]);
                    if (!value) {
                        value = new std::string("");
                    }
                    if (!value->empty()) {
                        response = *value;
                    } else {
                        response = "(nil)";
                    }
                    delete value;
                    success = true;
                } else {
                    response = "Invalid number of arguments for GET command";
                }
                break;
            }
            case CommandType::DEL: {
                if (parsed_cmd->args.size() >= 1) {
                    success = db_.del(parsed_cmd->args[0]);
                    if (success) {
                        aof_.append_command("DEL " + parsed_cmd->args[0]);
                        response = "1";
                    } else {
                        response = "0";
                    }
                } else {
                    response = "Invalid number of arguments for DEL command";
                }
                break;
            }
            case CommandType::HSET: {
                if (parsed_cmd->args.size() >= 3) {
                    success = db_.hset(parsed_cmd->args[0], parsed_cmd->args[1], parsed_cmd->args[2]);
                    if (success) {
                        aof_.append_command("HSET " + parsed_cmd->args[0] + " " + parsed_cmd->args[1] + " " + parsed_cmd->args[2]);
                        response = "1";
                    } else {
                        response = "0";
                    }
                } else {
                    response = "Invalid number of arguments for HSET command";
                }
                break;
            }
            case CommandType::HGET: {
                if (parsed_cmd->args.size() >= 2) {
                    std::string* value = db_.hget(parsed_cmd->args[0], parsed_cmd->args[1]);
                    if (!value) {
                        value = new std::string("");
                    }
                    if (!value->empty()) {
                        response = *value;
                    } else {
                        response = "(nil)";
                    }
                    delete value;
                    success = true;
                } else {
                    response = "Invalid number of arguments for HGET command";
                }
                break;
            }
            case CommandType::LPUSH: {
                if (parsed_cmd->args.size() >= 2) {
                    success = db_.lpush(parsed_cmd->args[0], parsed_cmd->args[1]);
                    if (success) {
                        aof_.append_command("LPUSH " + parsed_cmd->args[0] + " " + parsed_cmd->args[1]);
                        response = "1";
                    } else {
                        response = "0";
                    }
                } else {
                    response = "Invalid number of arguments for LPUSH command";
                }
                break;
            }
            case CommandType::LRANGE: {
                if (parsed_cmd->args.size() >= 3) {
                    int start = std::stoi(parsed_cmd->args[1]);
                    int end = std::stoi(parsed_cmd->args[2]);
                    std::vector<std::string>* list = db_.lrange(parsed_cmd->args[0], start, end);
                    if (!list) {
                        list = new std::vector<std::string>();
                    }
                    if (!list->empty()) {
                        std::ostringstream oss;
                        for (size_t i = 0; i < list->size(); ++i) {
                            oss << "[" << i << "]\"" << list->at(i) << "\"\n";
                        }
                        response = oss.str();
                    } else {
                        response = "(nil)";
                    }
                    delete list;
                    success = true;
                } else {
                    response = "Invalid number of arguments for LRANGE command";
                }
                break;
            }
            default: {
                response = "Unknown command";
                break;
            }
        }

        // 清理内存
        delete parsed_cmd;

        // 设置响应内容
        if (success) {
            res.set_content(response, "text/plain");
        } else {
            res.set_content(response, "text/plain");
            res.status = 400;
        }
    });

    // 处理根路径
    server.get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("MiniRedis HTTP Server\n\n" 
                         "Usage:\n" 
                         "  GET /cmd?cmd=SET key value [EX seconds]\n" 
                         "  GET /cmd?cmd=GET key\n" 
                         "  GET /cmd?cmd=DEL key\n" 
                         "  GET /cmd?cmd=HSET hash field value\n" 
                         "  GET /cmd?cmd=HGET hash field\n" 
                         "  GET /cmd?cmd=LPUSH list value\n" 
                         "  GET /cmd?cmd=LRANGE list 0 -1", 
                         "text/plain");
    });

    // 启动服务器
    std::cout << "HTTP server listening on port " << port_ << std::endl;
    if (!server.listen("0.0.0.0", port_)) {
        std::cerr << "Failed to start HTTP server" << std::endl;
        return;
    }

    // 等待服务器停止
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    server.stop();
}
