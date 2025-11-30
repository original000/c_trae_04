#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>

// 命令类型
enum class CommandType {
    SET,
    GET,
    DEL,
    HSET,
    HGET,
    LPUSH,
    LRANGE,
    UNKNOWN
};

// 解析后的命令
struct ParsedCommand {
    CommandType type;
    std::vector<std::string> args;
};

class CommandParser {
public:
    CommandParser() = default;
    ~CommandParser() = default;

    // 解析Redis协议命令
    ParsedCommand* parse_redis_protocol(const std::string& data);

    // 解析HTTP查询字符串命令（格式：cmd=SET%20key%20value）
    ParsedCommand* parse_http_query(const std::string& query);

public:
    // 解析空格分隔的命令字符串
    ParsedCommand* parse_plain_text(const std::string& text);

private:
    // 转换命令字符串到CommandType
    CommandType string_to_command_type(const std::string& str);
};

#endif // COMMAND_H
