#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <experimental/string_view>
#include <utility>
#include "Optional.h"

// 为了兼容性，使用std::experimental::string_view
using std::experimental::string_view;

// 为了方便，定义Optional的别名
using std::nullopt_t;
constexpr nullopt_t nullopt = nullopt_t{};

// 解析器类型定义
template <typename T>
using Parser = std::function<Optional<std::pair<T, string_view>>(string_view)>;

// 宏定义：快速创建解析器
#define PARSER_LAMBDA [](string_view input) -> Optional<std::pair<decltype(auto), string_view>>

// 基础解析器：take(n) - 取前n个字符
template <typename T = std::string_view>
Parser<T> take(size_t n) {
    return PARSER_LAMBDA {
        if (input.size() >= n) {
            return Optional<std::pair<T, string_view>>(std::make_pair(
                T(input.substr(0, n)),
                input.substr(n)
            ));
        }
        return Optional<std::pair<T, string_view>>();
    };
}

// 基础解析器：take_until - 取到指定字符串之前的内容
Parser<std::string_view> take_until(std::string_view delimiter) {
    return PARSER_LAMBDA {
        size_t pos = input.find(delimiter);
        if (pos != string_view::npos) {
            return Optional<std::pair<string_view, string_view>>(std::make_pair(
                input.substr(0, pos),
                input.substr(pos + delimiter.size())
            ));
        }
        return Optional<std::pair<string_view, string_view>>();
    };
}

// 基础解析器：tag - 匹配指定字符串
Parser<std::string_view> tag(std::string_view expected) {
    return PARSER_LAMBDA {
        if (input.size() >= expected.size() && 
            input.substr(0, expected.size()) == expected) {
            return Optional<std::pair<string_view, string_view>>(std::make_pair(
                expected,
                input.substr(expected.size())
            ));
        }
        return Optional<std::pair<string_view, string_view>>();
    };
}

// 基础解析器：digit - 匹配单个数字
Parser<char> digit() {
    return PARSER_LAMBDA {
        if (!input.empty() && isdigit(static_cast<unsigned char>(input[0]))) {
            return Optional<std::pair<char, string_view>>(std::make_pair(
                input[0],
                input.substr(1)
            ));
        }
        return Optional<std::pair<char, string_view>>();
    };
}

// 基础解析器：alpha - 匹配单个字母
Parser<char> alpha() {
    return PARSER_LAMBDA {
        if (!input.empty() && isalpha(static_cast<unsigned char>(input[0]))) {
            return Optional<std::pair<char, string_view>>(std::make_pair(
                input[0],
                input.substr(1)
            ));
        }
        return Optional<std::pair<char, string_view>>();
    };
}

// 基础解析器：hex_u32 - 解析十六进制字符串为uint32_t
Parser<uint32_t> hex_u32() {
    return PARSER_LAMBDA {
        uint32_t result = 0;
        size_t i = 0;
        while (i < input.size()) {
            char c = input[i];
            if (isdigit(static_cast<unsigned char>(c))) {
                result = result * 16 + (c - '0');
            } else if (c >= 'a' && c <= 'f') {
                result = result * 16 + (c - 'a' + 10);
            } else if (c >= 'A' && c <= 'F') {
                result = result * 16 + (c - 'A' + 10);
            } else {
                break;
            }
            i++;
        }
        if (i > 0) {
            return Optional<std::pair<uint32_t, string_view>>(std::make_pair(
                result,
                input.substr(i)
            ));
        }
        return Optional<std::pair<uint32_t, string_view>>();
    };
}

// 基础解析器：le_u16 - 解析小端字节序的uint16_t
Parser<uint16_t> le_u16() {
    return PARSER_LAMBDA {
        if (input.size() >= 2) {
            uint16_t result = static_cast<uint16_t>(
                static_cast<unsigned char>(input[0]) | 
                (static_cast<unsigned char>(input[1]) << 8)
            );
            return Optional<std::pair<uint16_t, string_view>>(std::make_pair(
                result,
                input.substr(2)
            ));
        }
        return Optional<std::pair<uint16_t, string_view>>();
    };
}

#endif // PARSER_H