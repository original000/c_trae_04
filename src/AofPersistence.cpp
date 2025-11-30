#include "AofPersistence.h"
#include "Database.h"
#include "Command.h"
#include <chrono>
#include <iostream>

AofPersistence::AofPersistence(Database& db, const std::string& filename)
    : db_(db), filename_(filename), running_(false) {
}

AofPersistence::~AofPersistence() {
    stop();
}

bool AofPersistence::start() {
    // 打开AOF文件（追加模式）
    file_.open(filename_, std::ios::out | std::ios::app | std::ios::binary);
    if (!file_.is_open()) {
        std::cerr << "Failed to open AOF file: " << filename_ << std::endl;
        return false;
    }

    // 启动fsync线程
    running_ = true;
    fsync_thread_ = std::thread(&AofPersistence::fsync_thread_func, this);

    return true;
}

void AofPersistence::stop() {
    if (running_) {
        running_ = false;
        if (fsync_thread_.joinable()) {
            fsync_thread_.join();
        }
    }

    if (file_.is_open()) {
        // 最后一次fsync
        file_.flush();
        file_.close();
    }
}

bool AofPersistence::append_command(const std::string& command) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!file_.is_open()) {
        return false;
    }

    // 写入命令（每行一个命令）
    file_ << command << "\n";

    return true;
}

bool AofPersistence::load() {
    // 关闭当前打开的文件（如果有的话）
    if (file_.is_open()) {
        file_.close();
    }

    // 打开AOF文件（只读模式）
    std::ifstream infile(filename_, std::ios::in | std::ios::binary);
    if (!infile.is_open()) {
        std::cerr << "Failed to open AOF file for loading: " << filename_ << std::endl;
        return false;
    }

    std::string line;
    CommandParser parser;

    // 逐行读取并执行命令
    while (std::getline(infile, line)) {
        if (line.empty()) {
            continue;
        }

        // 解析命令
        ParsedCommand* parsed_cmd = parser.parse_plain_text(line);
        if (!parsed_cmd) {
            std::cerr << "Failed to parse command from AOF: " << line << std::endl;
            continue;
        }

        // 执行命令
        bool success = false;
        switch (parsed_cmd->type) {
            case CommandType::SET: {
                if (parsed_cmd->args.size() >= 2) {
                    int expire_seconds = 0;
                    for (size_t i = 2; i < parsed_cmd->args.size(); ++i) {
                        if (parsed_cmd->args[i] == "EX" && i + 1 < parsed_cmd->args.size()) {
                            expire_seconds = std::stoi(parsed_cmd->args[i + 1]);
                            break;
                        }
                    }
                    success = db_.set(parsed_cmd->args[0], parsed_cmd->args[1], expire_seconds);
                }
                break;
            }
            case CommandType::DEL: {
                if (parsed_cmd->args.size() >= 1) {
                    success = db_.del(parsed_cmd->args[0]);
                }
                break;
            }
            case CommandType::HSET: {
                if (parsed_cmd->args.size() >= 3) {
                    success = db_.hset(parsed_cmd->args[0], parsed_cmd->args[1], parsed_cmd->args[2]);
                }
                break;
            }
            case CommandType::LPUSH: {
                if (parsed_cmd->args.size() >= 2) {
                    success = db_.lpush(parsed_cmd->args[0], parsed_cmd->args[1]);
                }
                break;
            }
            default:
                std::cerr << "Unknown command type from AOF: " << static_cast<int>(parsed_cmd->type) << std::endl;
                break;
        }

        if (!success) {
            std::cerr << "Failed to execute command from AOF: " << line << std::endl;
        }

        // 释放动态分配的ParsedCommand对象
        delete parsed_cmd;
    }

    infile.close();

    // 重新打开文件以追加模式
    return start();
}

void AofPersistence::fsync_thread_func() {
    while (running_) {
        // 每秒fsync一次
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::lock_guard<std::mutex> lock(mutex_);
        if (file_.is_open()) {
            file_.flush();
        }
    }
}
