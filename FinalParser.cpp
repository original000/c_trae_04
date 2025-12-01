#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <cstring>
#include <cstdint>

// 解析器类型定义
template <typename T>
using Parser = std::function<std::pair<bool, std::pair<T, std::string>>(const std::string&)>;

// 基础解析器：take - 取固定长度的字符
Parser<std::string> take_string(size_t n) {
    return [n](const std::string& input) {
        if (input.size() >= n) {
            std::string result = input.substr(0, n);
            std::string remaining = input.substr(n);
            return std::make_pair(true, std::make_pair(result, remaining));
        }
        return std::make_pair(false, std::make_pair(std::string(), input));
    };
}

// 基础解析器：take_until - 取字符直到遇到指定字符
Parser<std::string> take_until_string(char c) {
    return [c](const std::string& input) {
        size_t pos = input.find(c);
        if (pos != std::string::npos) {
            std::string result = input.substr(0, pos);
            std::string remaining = input.substr(pos);
            return std::make_pair(true, std::make_pair(result, remaining));
        }
        return std::make_pair(false, std::make_pair(std::string(), input));
    };
}

// 基础解析器：tag - 匹配指定的字符串
Parser<std::string> tag_string(const std::string& expected) {
    return [expected](const std::string& input) {
        if (input.size() >= expected.size() && input.substr(0, expected.size()) == expected) {
            std::string remaining = input.substr(expected.size());
            return std::make_pair(true, std::make_pair(expected, remaining));
        }
        return std::make_pair(false, std::make_pair(std::string(), input));
    };
}

// 基础解析器：le_u16 - 解析小端序的uint16_t
Parser<uint16_t> le_u16() {
    return [](const std::string& input) {
        if (input.size() >= 2) {
            uint16_t value = static_cast<uint16_t>(static_cast<unsigned char>(input[0])) | 
                              (static_cast<uint16_t>(static_cast<unsigned char>(input[1])) << 8);
            std::string remaining = input.substr(2);
            return std::make_pair(true, std::make_pair(value, remaining));
        }
        return std::make_pair(false, std::make_pair(static_cast<uint16_t>(0), input));
    };
}

// 组合器：seq - 按顺序应用两个解析器，返回结果对
Parser<std::pair<std::string, std::string>> seq_string_string(Parser<std::string>& a_parser, Parser<std::string>& b_parser) {
    return [a_parser, b_parser](const std::string& input) {
        auto a_result = a_parser(input);
        if (a_result.first) {
            auto b_result = b_parser(a_result.second.second);
            if (b_result.first) {
                std::pair<std::string, std::string> result_pair = std::make_pair(a_result.second.first, b_result.second.first);
                return std::make_pair(true, std::make_pair(result_pair, b_result.second.second));
            }
        }
        return std::make_pair(false, std::make_pair(std::make_pair(std::string(), std::string()), input));
    };
}

// 组合器：alt - 尝试第一个解析器，如果失败则尝试第二个
Parser<std::string> alt_string(Parser<std::string>& parser1, Parser<std::string>& parser2) {
    return [parser1, parser2](const std::string& input) {
        auto result1 = parser1(input);
        if (result1.first) {
            return result1;
        }
        return parser2(input);
    };
}

// 组合器：many0 - 零次或多次应用解析器，返回结果向量
Parser<std::vector<std::string>> many0_string(Parser<std::string>& parser) {
    return [parser](const std::string& input) {
        std::vector<std::string> results;
        std::string remaining = input;

        while (true) {
            auto result = parser(remaining);
            if (result.first) {
                results.push_back(result.second.first);
                remaining = result.second.second;
            } else {
                break;
            }
        }

        return std::make_pair(true, std::make_pair(results, remaining));
    };
}

// 组合器：separated_list - 解析由分隔符分隔的列表
Parser<std::vector<std::string>> separated_list_string(Parser<std::string>& item_parser, Parser<std::string>& separator_parser) {
    return [item_parser, separator_parser](const std::string& input) {
        auto first_result = item_parser(input);
        if (!first_result.first) {
            // 没有元素，返回空列表
            return std::make_pair(true, std::make_pair(std::vector<std::string>(), input));
        }

        std::vector<std::string> results;
        results.push_back(first_result.second.first);
        std::string remaining = first_result.second.second;

        while (true) {
            // 尝试解析分隔符
            auto separator_result = separator_parser(remaining);
            if (!separator_result.first) {
                break;
            }

            // 分隔符解析成功，尝试解析下一个元素
            remaining = separator_result.second.second;
            auto item_result = item_parser(remaining);
            if (!item_result.first) {
                // 分隔符后没有元素，解析失败
                return std::make_pair(false, std::make_pair(std::vector<std::string>(), input));
            }

            // 元素解析成功，添加到结果列表
            results.push_back(item_result.second.first);
            remaining = item_result.second.second;
        }

        return std::make_pair(true, std::make_pair(results, remaining));
    };
}

// 组合器：delimited - 解析由开始和结束标记包围的内容
Parser<std::string> delimited_string(Parser<std::string>& open_parser, Parser<std::string>& content_parser, Parser<std::string>& close_parser) {
    return [open_parser, content_parser, close_parser](const std::string& input) {
        // 解析开始标记
        auto open_result = open_parser(input);
        if (!open_result.first) {
            return std::make_pair(false, std::make_pair(std::string(), input));
        }

        // 解析内容
        std::string remaining_after_open = open_result.second.second;
        auto content_result = content_parser(remaining_after_open);
        if (!content_result.first) {
            return std::make_pair(false, std::make_pair(std::string(), input));
        }

        // 解析结束标记
        std::string remaining_after_content = content_result.second.second;
        auto close_result = close_parser(remaining_after_content);
        if (!close_result.first) {
            return std::make_pair(false, std::make_pair(std::string(), input));
        }

        // 所有部分都解析成功，返回内容
        return std::make_pair(true, std::make_pair(content_result.second.first, close_result.second.second));
    };
}

// 组合器：map - 转换解析器的结果
Parser<std::string> map_string_to_string(Parser<std::string>& parser, std::function<std::string(std::string)> transform) {
    return [parser, transform](const std::string& input) {
        auto result = parser(input);
        if (result.first) {
            std::string transformed_value = transform(result.second.first);
            return std::make_pair(true, std::make_pair(transformed_value, result.second.second));
        }
        return std::make_pair(false, std::make_pair(std::string(), input));
    };
}

// 辅助函数：打印解析结果
void print_result(const std::pair<bool, std::pair<std::string, std::string>>& result) {
    if (result.first) {
        std::cout << "Parsing succeeded! Result: \"" << result.second.first << "\"";
        std::cout << ", Remaining input: \"" << result.second.second << "\"" << std::endl;
    } else {
        std::cout << "Parsing failed!" << std::endl;
    }
}

void print_result(const std::pair<bool, std::pair<uint16_t, std::string>>& result) {
    if (result.first) {
        std::cout << "Parsing succeeded! Result: " << result.second.first;
        std::cout << ", Remaining input: \"" << result.second.second << "\"" << std::endl;
    } else {
        std::cout << "Parsing failed!" << std::endl;
    }
}

void print_result(const std::pair<bool, std::pair<std::vector<std::string>, std::string>>& result) {
    if (result.first) {
        std::cout << "Parsing succeeded! Result: [ ";
        for (size_t i = 0; i < result.second.first.size(); ++i) {
            std::cout << "\"" << result.second.first[i] << "\" ";
        }
        std::cout << "]";
        std::cout << ", Remaining input: \"" << result.second.second << "\"" << std::endl;
    } else {
        std::cout << "Parsing failed!" << std::endl;
    }
}

void print_result(const std::pair<bool, std::pair<std::pair<std::string, std::string>, std::string>>& result) {
    if (result.first) {
        std::cout << "Parsing succeeded! Result: (\"" << result.second.first.first << "\", \"" << result.second.first.second << "\")";
        std::cout << ", Remaining input: \"" << result.second.second << "\"" << std::endl;
    } else {
        std::cout << "Parsing failed!" << std::endl;
    }
}

int main() {
    // Example 1: Parse fixed-length string using take
    std::cout << "Example 1: take_string(5)" << std::endl;
    auto take_parser = take_string(5);
    auto result1 = take_parser("Hello, World!");
    print_result(result1);
    std::cout << std::endl;

    // Example 2: Match specific string using tag
    std::cout << "Example 2: tag_string(\"Hello\")" << std::endl;
    auto tag_parser = tag_string("Hello");
    auto result2 = tag_parser("Hello, World!");
    print_result(result2);
    std::cout << std::endl;

    // Example 3: Combine two parsers in sequence using seq
    std::cout << "Example 3: seq_string_string(take_string(5), tag_string(\", \"))" << std::endl;
    auto take_parser3 = take_string(5);
    auto tag_parser3 = tag_string(", ");
    auto seq_parser = seq_string_string(take_parser3, tag_parser3);
    auto result3 = seq_parser("Hello, World!");
    print_result(result3);
    std::cout << std::endl;

    // Example 4: Try two parsers in alternative using alt
    std::cout << "Example 4: alt_string(tag_string(\"Hello\"), tag_string(\"Hi\"))" << std::endl;
    auto tag_parser4a = tag_string("Hello");
    auto tag_parser4b = tag_string("Hi");
    auto alt_parser = alt_string(tag_parser4a, tag_parser4b);
    auto result4 = alt_parser("Hi, World!");
    print_result(result4);
    std::cout << std::endl;

    // Example 5: Apply parser zero or more times using many0
    std::cout << "Example 5: many0_string(tag_string(\"ab\"))" << std::endl;
    auto tag_parser5 = tag_string("ab");
    auto many0_parser = many0_string(tag_parser5);
    auto result5 = many0_parser("abababxyz");
    print_result(result5);
    std::cout << std::endl;

    // Example 6: Parse list separated by delimiter using separated_list
    std::cout << "Example 6: separated_list_string(tag_string(\"a\"), tag_string(\",\"))" << std::endl;
    auto tag_parser6a = tag_string("a");
    auto tag_parser6b = tag_string(",");
    auto separated_list_parser = separated_list_string(tag_parser6a, tag_parser6b);
    auto result6 = separated_list_parser("a,a,a");
    print_result(result6);
    std::cout << std::endl;

    // Example 7: Parse content delimited by start and end tags using delimited
    std::cout << "Example 7: delimited_string(tag_string(\"(\"), take_until_string(')'), tag_string(\")\"))" << std::endl;
    auto tag_parser7a = tag_string("(");
    auto take_until_parser7 = take_until_string(')');
    auto tag_parser7b = tag_string(")");
    auto delimited_parser = delimited_string(tag_parser7a, take_until_parser7, tag_parser7b);
    auto result7 = delimited_parser("(Hello, World!)xyz");
    print_result(result7);
    std::cout << std::endl;

    // Example 8: Transform parser result using map
    std::cout << "Example 8: map_string_to_string(take_string(5), [](std::string s) { return s + s; })" << std::endl;
    auto take_parser8 = take_string(5);
    auto map_parser = map_string_to_string(take_parser8, [](std::string s) { return s + s; });
    auto result8 = map_parser("Hello, World!");
    print_result(result8);
    std::cout << std::endl;

    return 0;
}