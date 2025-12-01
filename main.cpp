#include "Parser.h"
#include "Combinators.h"
#include <iomanip> // for std::setw and std::setfill

// 导入解析器函数和组合器
using namespace std;

// 声明解析器函数和组合器
template <typename T = string_view>
Parser<T> take(size_t n);

Parser<string_view> take_until(string_view delimiter);
Parser<string_view> tag(string_view expected);
Parser<char> digit();
Parser<char> alpha();
Parser<uint32_t> hex_u32();
Parser<uint16_t> le_u16();

template <typename... Parsers>
auto seq(Parsers&&... parsers);

template <typename... Parsers>
auto alt(Parsers&&... parsers);

template <typename Parser>
Parser<vector<typename Parser::result_type::value_type::first_type>> many0(Parser&& parser);

template <typename Parser>
Parser<vector<typename Parser::result_type::value_type::first_type>> many1(Parser&& parser);

template <typename Parser, typename SeparatorParser>
Parser<vector<typename Parser::result_type::value_type::first_type>> separated_list(Parser&& parser, SeparatorParser&& separator);

template <typename OpenParser, typename ContentParser, typename CloseParser>
Parser<typename ContentParser::result_type::value_type::first_type> delimited(OpenParser&& open, ContentParser&& content, CloseParser&& close);

template <typename PreParser, typename ContentParser>
Parser<typename ContentParser::result_type::value_type::first_type> preceded(PreParser&& pre, ContentParser&& content);

template <typename ContentParser, typename PostParser>
Parser<typename ContentParser::result_type::value_type::first_type> terminated(ContentParser&& content, PostParser&& post);

template <typename Parser, typename Transform>
Parser<decltype(declval<Transform>()(declval<typename Parser::result_type::value_type::first_type>()))> map(Parser&& parser, Transform&& transform);

template <typename Parser, typename Transform>
Parser<typename result_of<Transform(typename Parser::result_type::value_type::first_type)>::type::result_type::value_type::first_type> flat_map(Parser&& parser, Transform&& transform);
#include <iostream>
#include <chrono>
#include <vector>

// 辅助函数：打印解析结果
template <typename T>
void print_result(const Optional<std::pair<T, string_view>>& result) {
    if (result) {
        std::cout << "解析成功: ";
        // 简单的类型推导打印
        if (std::is_same<T, string_view>::value) {
            std::cout << "\"" << result->first << "\"";
        } else if (std::is_same<T, char>::value) {
            std::cout << "'" << result->first << "'";
        } else if (std::is_integral<T>::value) {
            std::cout << result->first;
        } else if (std::is_same<T, std::vector<char>>::value) {
            std::cout << "[ ";
            for (char c : result->first) {
                std::cout << "'" << c << "' ";
            }
            std::cout << "]";
        } else if (std::is_same<T, std::vector<uint32_t>>::value) {
            std::cout << "[ ";
            for (uint32_t n : result->first) {
                std::cout << n << " ";
            }
            std::cout << "]";
        } else {
            std::cout << "(复杂类型)";
        }
        
        if (!result->second.empty()) {
            std::cout << ", 剩余: \"" << result->second.substr(0, 20) << (result->second.size() > 20 ? "...\"" : "\"");
        }
        std::cout << std::endl;
    } else {
        std::cout << "解析失败" << std::endl;
    }
}

int main() {
    std::cout << "=== 零拷贝字节流解析器测试 ===\n\n";
    
    // 1. Redis RESP 批量字符串解析
    std::cout << "1. Redis RESP 批量字符串解析:\n";
    string_view redis_input = "$11\r\nhello world\r\n";
    
    // 解析Redis批量字符串：$<length>\r\n<data>\r\n
    // 首先解析$符号
    auto redis_parser = preceded(
        tag("$"),
        // 然后解析长度数字
        flat_map(
            many1(digit()),
            [](const std::vector<char>& digits) {
                // 将数字字符转换为整数
                size_t length = 0;
                for (char c : digits) {
                    length = length * 10 + (c - '0');
                }
                // 然后解析\r\n
                auto crlf_parser = tag("\r\n");
                
                // 然后解析指定长度的数据
                auto data_parser = take(length);
                
                // 最后解析结尾的\r\n
                return terminated(
                    preceded(crlf_parser, data_parser),
                    crlf_parser
                );
            }
        )
    );
    
    auto redis_result = redis_parser(redis_input);
    print_result(redis_result);
    
    std::cout << std::endl;
    
    // 2. HTTP/1.1 简单请求头解析
    std::cout << "2. HTTP/1.1 简单请求头解析:\n";
    string_view http_input = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
    
    // 解析HTTP请求行：METHOD PATH HTTP/VERSION\r\n
    auto method_parser = take_until(" ");
    auto path_parser = preceded(tag(" "), take_until(" "));
    auto version_parser = preceded(tag(" "), terminated(take_until("\r\n"), tag("\r\n")));
    
    auto request_line_parser = seq(method_parser, path_parser, version_parser);
    
    auto http_result = request_line_parser(http_input);
    if (http_result) {
        std::cout << "解析成功: " << std::endl;
        std::cout << "  方法: \"" << std::get<0>(http_result->first) << "\"" << std::endl;
        std::cout << "  路径: \"" << std::get<1>(http_result->first) << "\"" << std::endl;
        std::cout << "  版本: \"" << std::get<2>(http_result->first) << "\"" << std::endl;
        if (!http_result->second.empty()) {
            std::cout << "  剩余: \"" << http_result->second.substr(0, 40) << (http_result->second.size() > 40 ? "...\"" : "\"") << std::endl;
        }
    } else {
        std::cout << "解析失败" << std::endl;
    }
    
    std::cout << std::endl;
    
    // 3. 自定义二进制协议解析
    std::cout << "3. 自定义二进制协议解析:\n";
    // 协议格式：[魔数0x1234][版本号0x01][数据长度0x0005][数据0x0102030405]
    std::string binary_data = "\x12\x34\x01\x00\x05\x01\x02\x03\x04\x05";
    string_view binary_input = binary_data;
    
    // 解析魔数（0x1234）
    auto magic_parser = tag(string_view("\x12\x34", 2));
    
    // 解析版本号（1字节）
    auto version_parser = take<char>(1);
    
    // 解析数据长度（小端uint16_t）
    auto length_parser = le_u16();
    
    // 解析数据（指定长度的字节）
    auto data_parser = flat_map(
        length_parser,
        [](uint16_t length) {
            return take<string_view>(length);
        }
    );
    
    // 组合完整的协议解析器
    auto binary_parser = seq(magic_parser, version_parser, data_parser);
    
    auto binary_result = binary_parser(binary_input);
    if (binary_result) {
        std::cout << "解析成功: " << std::endl;
        std::cout << "  魔数: 0x1234" << std::endl;
        std::cout << "  版本: 0x" << std::hex << static_cast<unsigned int>(static_cast<unsigned char>(std::get<1>(binary_result->first)))) << std::dec << std::endl;
        std::cout << "  数据: 0x";
        string_view data = std::get<2>(binary_result->first);
        for (char c : data) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(static_cast<unsigned char>(c));
        }
        std::cout << std::dec << std::endl;
    } else {
        std::cout << "解析失败" << std::endl;
    }
    
    std::cout << std::endl;
    
    // 4. 压测：1万次 take(4) + tag("PING") 组合
    std::cout << "4. 压测：1万次 take(4) + tag(\"PING\") 组合:\n";
    
    // 创建测试输入数据
    std::string ping_data;
    for (int i = 0; i < 10000; ++i) {
        ping_data += "1234PING";
    }
    string_view ping_input = ping_data;
    
    // 创建解析器：take(4) 后跟 tag("PING")
    auto ping_parser = seq(take(4), tag("PING"));
    
    // 开始压测
    auto start_time = std::chrono::high_resolution_clock::now();
    
    string_view current_input = ping_input;
    int count = 0;
    while (true) {
        auto result = ping_parser(current_input);
        if (!result) {
            break;
        }
        count++;
        current_input = result->second;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "  解析次数: " << count << std::endl;
    std::cout << "  总耗时: " << duration.count() << " 微秒" << std::endl;
    std::cout << "  平均耗时: " << (duration.count() / static_cast<double>(count)) << " 微秒/次" << std::endl;
    
    std::cout << std::endl;
    
    std::cout << "=== 所有测试完成 ===\n";
    
    return 0;
}
