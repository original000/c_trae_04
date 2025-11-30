#include "Command.h"
#include <sstream>
#include <algorithm>
#include <cctype>

// 解析Redis协议命令
ParsedCommand* CommandParser::parse_redis_protocol(const std::string& data) {
    std::istringstream iss(data);
    std::string line;

    // 读取数组长度
    if (!std::getline(iss, line)) {
        return nullptr;
    }

    if (line.empty() || line[0] != '*') {
        return nullptr;
    }

    int array_length = std::stoi(line.substr(1));
    if (array_length <= 0) {
        return nullptr;
    }

    std::vector<std::string> args;

    for (int i = 0; i < array_length; ++i) {
        // 读取字符串长度
        if (!std::getline(iss, line)) {
            return nullptr;
        }

        if (line.empty() || line[0] != '$') {
            return nullptr;
        }

        int str_length = std::stoi(line.substr(1));
        if (str_length < 0) {
            return nullptr;
        }

        // 读取字符串内容
        std::string str;
        str.resize(str_length);

        iss.read(&str[0], str_length);
        if (iss.gcount() != str_length) {
            return nullptr;
        }

        // 读取并忽略换行符
        std::getline(iss, line);

        args.push_back(str);
    }

    if (args.empty()) {
        return nullptr;
    }

    ParsedCommand* cmd = new ParsedCommand();
    cmd->type = string_to_command_type(args[0]);
    cmd->args = std::vector<std::string>(args.begin() + 1, args.end());

    return cmd;
}

// 解析HTTP查询字符串命令（格式：cmd=SET%20key%20value）
ParsedCommand* CommandParser::parse_http_query(const std::string& query) {
    // 查找cmd参数
    std::string cmd_param = "cmd=";
    size_t start = query.find(cmd_param);
    if (start == std::string::npos) {
        return nullptr;
    }

    start += cmd_param.length();
    size_t end = query.find('&', start);
    if (end == std::string::npos) {
        end = query.length();
    }

    // 提取并解码命令字符串
    std::string encoded_cmd = query.substr(start, end - start);
    std::string decoded_cmd;

    for (size_t i = 0; i < encoded_cmd.length(); ++i) {
        if (encoded_cmd[i] == '%') {
            if (i + 2 < encoded_cmd.length()) {
                std::string hex = encoded_cmd.substr(i + 1, 2);
                char c = static_cast<char>(std::stoi(hex, nullptr, 16));
                decoded_cmd += c;
                i += 2;
            }
        } else if (encoded_cmd[i] == '+') {
            decoded_cmd += ' ';
        } else {
            decoded_cmd += encoded_cmd[i];
        }
    }

    // 解析解码后的命令字符串
    return parse_plain_text(decoded_cmd);
}

// 解析空格分隔的命令字符串
ParsedCommand* CommandParser::parse_plain_text(const std::string& text) {
    std::istringstream iss(text);
    std::vector<std::string> args;
    std::string arg;

    while (iss >> arg) {
        args.push_back(arg);
    }

    if (args.empty()) {
        return nullptr;
    }

    ParsedCommand* cmd = new ParsedCommand();
    cmd->type = string_to_command_type(args[0]);
    cmd->args = std::vector<std::string>(args.begin() + 1, args.end());

    return cmd;
}

// 转换命令字符串到CommandType
CommandType CommandParser::string_to_command_type(const std::string& str) {
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(), ::toupper);

    if (upper_str == "SET") return CommandType::SET;
    if (upper_str == "GET") return CommandType::GET;
    if (upper_str == "DEL") return CommandType::DEL;
    if (upper_str == "HSET") return CommandType::HSET;
    if (upper_str == "HGET") return CommandType::HGET;
    if (upper_str == "LPUSH") return CommandType::LPUSH;
    if (upper_str == "LRANGE") return CommandType::LRANGE;

    return CommandType::UNKNOWN;
}
