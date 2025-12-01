#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <cstdint>
#include <cstring>
#include <iomanip>

// 为了兼容C++14，我们使用自定义的Optional类型
// 以及std::experimental::string_view或自定义的string_view

// 自定义string_view，兼容C++14
class string_view {
public:
    string_view() : data_(nullptr), size_(0) {}
    string_view(const char* data) : data_(data), size_(std::strlen(data)) {}
    string_view(const char* data, size_t size) : data_(data), size_(size) {}
    string_view(const std::string& str) : data_(str.data()), size_(str.size()) {}
    
    const char* data() const { return data_; }
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    
    char operator[](size_t index) const { return data_[index]; }
    
    string_view substr(size_t pos, size_t count = npos) const {
        if (pos > size_) pos = size_;
        if (count > size_ - pos) count = size_ - pos;
        return string_view(data_ + pos, count);
    }
    
    size_t find(char c, size_t pos = 0) const {
        for (size_t i = pos; i < size_; ++i) {
            if (data_[i] == c) return i;
        }
        return npos;
    }
    
    bool starts_with(const string_view& prefix) const {
        if (prefix.size_ > size_) return false;
        for (size_t i = 0; i < prefix.size_; ++i) {
            if (data_[i] != prefix.data_[i]) return false;
        }
        return true;
    }
    
    static constexpr size_t npos = static_cast<size_t>(-1);
    
private:
    const char* data_;
    size_t size_;
};

// 自定义Optional类型，兼容C++14
template <typename T>
class Optional {
public:
    Optional() : has_value_(false) {}
    Optional(const T& value) : has_value_(true), value_(value) {}
    Optional(T&& value) : has_value_(true), value_(std::move(value)) {}
    
    Optional& operator=(const T& value) {
        has_value_ = true;
        value_ = value;
        return *this;
    }
    
    Optional& operator=(T&& value) {
        has_value_ = true;
        value_ = std::move(value);
        return *this;
    }
    
    explicit operator bool() const { return has_value_; }
    
    const T& operator*() const { return value_; }
    T& operator*() { return value_; }
    
    const T* operator->() const { return &value_; }
    T* operator->() { return &value_; }
    
    const T& value() const { return value_; }
    T& value() { return value_; }
    
private:
    bool has_value_;
    T value_;
};

// 定义Parser类型
template <typename T>
using Parser = std::function<Optional<std::pair<T, string_view>>(string_view)>;

// 基础解析器：take - 读取指定数量的字符
template <typename T>
Parser<T> take(size_t n) {
    return [n](string_view input) -> Optional<std::pair<T, string_view>> {
        if (input.size() >= n) {
            if (std::is_same<T, string_view>::value) {
                return std::make_pair(input.substr(0, n), input.substr(n));
            } else if (std::is_same<T, char>::value) {
                return std::make_pair(input[0], input.substr(1));
            } else if (std::is_same<T, std::vector<char>>::value) {
                std::vector<char> result;
                for (size_t i = 0; i < n; ++i) {
                    result.push_back(input[i]);
                }
                return std::make_pair(result, input.substr(n));
            }
        }
        return Optional<std::pair<T, string_view>>();
    };
}

// 基础解析器：take_until - 读取直到遇到指定字符
template <typename T>
Parser<T> take_until(char c) {
    return [c](string_view input) -> Optional<std::pair<T, string_view>> {
        size_t pos = input.find(c);
        if (pos != string_view::npos) {
            if (std::is_same<T, string_view>::value) {
                return std::make_pair(input.substr(0, pos), input.substr(pos));
            } else if (std::is_same<T, std::vector<char>>::value) {
                std::vector<char> result;
                for (size_t i = 0; i < pos; ++i) {
                    result.push_back(input[i]);
                }
                return std::make_pair(result, input.substr(pos));
            }
        }
        return Optional<std::pair<T, string_view>>();
    };
}

// 基础解析器：tag - 匹配指定的字符串
template <typename T>
Parser<T> tag(const string_view& expected) {
    return [expected](string_view input) -> Optional<std::pair<T, string_view>> {
        if (input.size() >= expected.size() && input.substr(0, expected.size()) == expected) {
            if (std::is_same<T, string_view>::value) {
                return std::make_pair(expected, input.substr(expected.size()));
            } else if (std::is_same<T, std::vector<char>>::value) {
                std::vector<char> result;
                for (size_t i = 0; i < expected.size(); ++i) {
                    result.push_back(expected[i]);
                }
                return std::make_pair(result, input.substr(expected.size()));
            }
        }
        return Optional<std::pair<T, string_view>>();
    };
}

// 基础解析器：le_u16 - 解析小端序的16位无符号整数
Parser<uint16_t> le_u16() {
    return [](string_view input) -> Optional<std::pair<uint16_t, string_view>> {
        if (input.size() >= 2) {
            uint16_t value;
            std::memcpy(&value, input.data(), 2);
            return std::make_pair(value, input.substr(2));
        }
        return Optional<std::pair<uint16_t, string_view>>();
    };
}

// 组合器：seq - 按顺序应用两个解析器

template <typename A, typename B>
Parser<std::pair<A, B>> seq(Parser<A>&& a_parser, Parser<B>&& b_parser) {
    return [a_parser = std::forward<Parser<A>>(a_parser), b_parser = std::forward<Parser<B>>(b_parser)](string_view input) -> Optional<std::pair<std::pair<A, B>, string_view>> {
        auto a_result = a_parser(input);
        if (a_result) {
            auto b_result = b_parser(a_result->second);
            if (b_result) {
                return std::make_pair(std::make_pair(a_result->first, b_result->first), b_result->second);
            }
        }
        return Optional<std::pair<std::pair<A, B>, string_view>>();
    };
}

// 组合器：alt - 尝试第一个解析器，如果失败则尝试第二个

template <typename A>
Parser<A> alt(Parser<A>&& a_parser, Parser<A>&& b_parser) {
    return [a_parser = std::forward<Parser<A>>(a_parser), b_parser = std::forward<Parser<A>>(b_parser)](string_view input) -> Optional<std::pair<A, string_view>> {
        auto a_result = a_parser(input);
        if (a_result) {
            return a_result;
        }
        return b_parser(input);
    };
}

// 组合器：many0 - 零次或多次应用解析器
template <typename A>
Parser<std::vector<A>> many0(Parser<A>&& parser) {
    return [parser = std::forward<Parser<A>>(parser)](string_view input) -> Optional<std::pair<std::vector<A>, string_view>> {
        std::vector<A> results;
        string_view remaining = input;
        
        while (true) {
            auto result = parser(remaining);
            if (result) {
                results.push_back(result->first);
                remaining = result->second;
            } else {
                break;
            }
        }
        
        return std::make_pair(results, remaining);
    };
}

// 组合器：many1 - 一次或多次应用解析器
template <typename A>
Parser<std::vector<A>> many1(Parser<A>&& parser) {
    return [parser = std::forward<Parser<A>>(parser)](string_view input) -> Optional<std::pair<std::vector<A>, string_view>> {
        std::vector<A> results;
        string_view remaining = input;
        
        // 至少需要一次成功
        auto first_result = parser(remaining);
        if (!first_result) {
            return Optional<std::pair<std::vector<A>, string_view>>();
        }
        
        results.push_back(first_result->first);
        remaining = first_result->second;
        
        // 零次或多次额外应用
        while (true) {
            auto result = parser(remaining);
            if (result) {
                results.push_back(result->first);
                remaining = result->second;
            } else {
                break;
            }
        }
        
        return std::make_pair(results, remaining);
    };
}

// 组合器：separated_list - 解析由分隔符分隔的列表
template <typename A, typename Sep>
Parser<std::vector<A>> separated_list(Parser<A>&& item_parser, Parser<Sep>&& sep_parser) {
    return [item_parser = std::forward<Parser<A>>(item_parser), sep_parser = std::forward<Parser<Sep>>(sep_parser)](string_view input) -> Optional<std::pair<std::vector<A>, string_view>> {
        std::vector<A> results;
        string_view remaining = input;
        
        // 解析第一个元素（可选）
        auto first_result = item_parser(remaining);
        if (first_result) {
            results.push_back(first_result->first);
            remaining = first_result->second;
            
            // 解析分隔符和后续元素
            while (true) {
                auto sep_result = sep_parser(remaining);
                if (sep_result) {
                    auto item_result = item_parser(sep_result->second);
                    if (item_result) {
                        results.push_back(item_result->first);
                        remaining = item_result->second;
                    } else {
                        // 分隔符后面没有有效的元素，解析失败
                        return Optional<std::pair<std::vector<A>, string_view>>();
                    }
                } else {
                    // 没有更多的分隔符，解析成功
                    break;
                }
            }
        }
        
        return std::make_pair(results, remaining);
    };
}

// 组合器：delimited - 解析由前缀和后缀包围的内容
template <typename Prefix, typename A, typename Suffix>
Parser<A> delimited(Parser<Prefix>&& prefix_parser, Parser<A>&& item_parser, Parser<Suffix>&& suffix_parser) {
    return [prefix_parser = std::forward<Parser<Prefix>>(prefix_parser), item_parser = std::forward<Parser<A>>(item_parser), suffix_parser = std::forward<Parser<Suffix>>(suffix_parser)](string_view input) -> Optional<std::pair<A, string_view>> {
        auto prefix_result = prefix_parser(input);
        if (prefix_result) {
            auto item_result = item_parser(prefix_result->second);
            if (item_result) {
                auto suffix_result = suffix_parser(item_result->second);
                if (suffix_result) {
                    return std::make_pair(item_result->first, suffix_result->second);
                }
            }
        }
        return Optional<std::pair<A, string_view>>();
    };
}

// 组合器：preceded - 解析前缀，然后解析内容并返回内容
template <typename Prefix, typename A>
Parser<A> preceded(Parser<Prefix>&& prefix_parser, Parser<A>&& item_parser) {
    return [prefix_parser = std::forward<Parser<Prefix>>(prefix_parser), item_parser = std::forward<Parser<A>>(item_parser)](string_view input) -> Optional<std::pair<A, string_view>> {
        auto prefix_result = prefix_parser(input);
        if (prefix_result) {
            return item_parser(prefix_result->second);
        }
        return Optional<std::pair<A, string_view>>();
    };
}

// 组合器：terminated - 解析内容，然后解析后缀并返回内容
template <typename A, typename Suffix>
Parser<A> terminated(Parser<A>&& item_parser, Parser<Suffix>&& suffix_parser) {
    return [item_parser = std::forward<Parser<A>>(item_parser), suffix_parser = std::forward<Parser<Suffix>>(suffix_parser)](string_view input) -> Optional<std::pair<A, string_view>> {
        auto item_result = item_parser(input);
        if (item_result) {
            auto suffix_result = suffix_parser(item_result->second);
            if (suffix_result) {
                return std::make_pair(item_result->first, suffix_result->second);
            }
        }
        return Optional<std::pair<A, string_view>>();
    };
}

// 组合器：map - 转换解析器的结果
template <typename A, typename B>
Parser<B> map(Parser<A>&& parser, std::function<B(A)>&& transform) {
    return [parser = std::forward<Parser<A>>(parser), transform = std::forward<std::function<B(A)>>(transform)](string_view input) -> Optional<std::pair<B, string_view>> {
        auto result = parser(input);
        if (result) {
            return std::make_pair(transform(result->first), result->second);
        }
        return Optional<std::pair<B, string_view>>();
    };
}

// 组合器：flat_map - 扁平转换解析器结果
template <typename ParserType, typename Transform>
auto flat_map(ParserType&& parser, Transform&& transform) {
    using InputType = typename decltype(parser(string_view()))::value_type::first_type;
    using OutputParser = typename std::result_of<Transform(InputType)>::type;
    using OutputType = typename decltype(std::declval<OutputParser>()(string_view()))::value_type::first_type;
    
    return [parser = std::forward<ParserType>(parser), transform = std::forward<Transform>(transform)](string_view input) -> Optional<std::pair<OutputType, string_view>> {
        auto result = parser(input);
        if (result) {
            auto output_parser = transform(result->first);
            return output_parser(result->second);
        }
        return Optional<std::pair<OutputType, string_view>>();
    };
}

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
            std::cout << ", 剩余输入: \"" << result->second << "\"";
        }
        std::cout << std::endl;
    } else {
        std::cout << "解析失败" << std::endl;
    }
}

int main() {
    // 示例1: Redis RESP批量字符串解析器
    std::cout << "示例1: Redis RESP批量字符串解析器" << std::endl;
    
    auto resp_bulk_string_parser = terminated(
        preceded(
            tag<char>($"$"$),
            take_until<char>($'\r'$)
        ),
        tag<char>($"\r\n"$)
    );
    
    string_view resp_input = $"$hello world\r\n"$;
    auto resp_result = resp_bulk_string_parser(resp_input);
    print_result(resp_result);
    
    // 示例2: 二进制协议解析器
    std::cout << "\n示例2: 二进制协议解析器" << std::endl;
    
    // 协议格式: [魔数(2字节)][版本(1字节)][长度(2字节)][数据(长度字节)]
    
    // 魔数解析器: 固定值0x1234
    auto magic_parser = tag<char>(string_view(reinterpret_cast<const char*>(&(uint16_t){0x1234}), 2));
    
    // 版本解析器: 1字节
    auto version_parser = take<char>(1);
    
    // 长度解析器: 小端序16位整数
    auto length_parser = le_u16();
    
    // 数据解析器: 根据长度解析指定数量的字节
    auto data_parser = flat_map<uint16_t, Parser<std::vector<char>>>(
        length_parser,
        [](uint16_t length) { return take<std::vector<char>>(length); }
    );
    
    // 完整的协议解析器
    auto binary_protocol_parser = seq(
        seq(
            magic_parser,
            version_parser
        ),
        data_parser
    );
    
    // 构造二进制输入数据
    uint16_t magic = 0x1234;
    char version = 1;
    uint16_t length = 5;
    char data[] = $"hello"$;
    
    std::vector<char> binary_input;
    binary_input.insert(binary_input.end(), reinterpret_cast<char*>(&magic), reinterpret_cast<char*>(&magic) + 2);
    binary_input.push_back(version);
    binary_input.insert(binary_input.end(), reinterpret_cast<char*>(&length), reinterpret_cast<char*>(&length) + 2);
    binary_input.insert(binary_input.end(), data, data + length);
    
    string_view binary_input_view(binary_input.data(), binary_input.size());
    auto binary_result = binary_protocol_parser(binary_input_view);
    print_result(binary_result);
    
    // 示例3: 简单配置文件解析器
    std::cout << "\n示例3: 简单配置文件解析器" << std::endl;
    
    // 配置文件格式:
    // key1=value1
    // key2=value2
    // ...
    
    // 解析键: 非等号字符
    auto key_parser = take_until<char>($'='$$);
    
    // 解析值: 非换行符字符
    auto value_parser = take_until<char>($'\n'$);
    
    // 解析键值对
    auto key_value_parser = seq(
        key_parser,
        preceded(
            tag<char>($"="$),
            value_parser
        )
    );
    
    // 解析配置文件: 多个键值对，每个键值对以换行符结束
    auto config_parser = many0(
        terminated(
            key_value_parser,
            alt(
                tag<char>($"\n"$),
                tag<char>($""$)  // 允许文件以键值对结束，没有换行符
            )
        )
    );
    
    string_view config_input = $"key1=value1\nkey2=value2\nkey3=value3"$;
    auto config_result = config_parser(config_input);
    print_result(config_result);
    
    return 0;
}