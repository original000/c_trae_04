#include <iostream>
#include <vector>
#include <functional>
#include <cstring>
#include <cstdint>
#include <type_traits>

// 自定义string_view类型
template <typename CharT>
struct basic_string_view {
    const CharT* data;
    size_t size;

    basic_string_view(const CharT* str, size_t len) : data(str), size(len) {}
    basic_string_view(const CharT* str) : data(str), size(strlen(str)) {}

    CharT operator[](size_t i) const { return data[i]; }
    const CharT* begin() const { return data; }
    const CharT* end() const { return data + size; }
    bool empty() const { return size == 0; }

    // 比较运算符
    bool operator==(const basic_string_view<CharT>& other) const {
        if (size != other.size) return false;
        return memcmp(data, other.data, size) == 0;
    }

    bool operator!=(const basic_string_view<CharT>& other) const {
        return !(*this == other);
    }

    // 子字符串
    basic_string_view<CharT> substr(size_t pos, size_t len) const {
        return basic_string_view<CharT>(data + pos, len);
    }
};

using string_view = basic_string_view<char>;

// 为string_view添加ostream运算符重载
template <typename CharT>
std::ostream& operator<<(std::ostream& os, const basic_string_view<CharT>& sv) {
    os.write(sv.data, sv.size);
    return os;
}

// 特化char版本的ostream运算符重载
std::ostream& operator<<(std::ostream& os, const string_view& sv) {
    os.write(sv.data, sv.size);
    return os; 
}

// 自定义Optional类型，兼容C++14
template <typename T>
class Optional {
private:
    union {
        T value;
    };
    bool has_value_;

public:
    Optional() : has_value_(false) {}
    Optional(const T& val) : value(val), has_value_(true) {}
    Optional(T&& val) : value(std::move(val)), has_value_(true) {}
    ~Optional() {
        if (has_value_) {
            value.~T();
        }
    }

    Optional(const Optional& other) : has_value_(other.has_value_) {
        if (has_value_) {
            new (&value) T(other.value);
        }
    }

    Optional(Optional&& other) : has_value_(other.has_value_) {
        if (has_value_) {
            new (&value) T(std::move(other.value));
        }
        other.has_value_ = false;
    }

    Optional& operator=(const Optional& other) {
        if (this != &other) {
            if (has_value_) {
                value.~T();
            }
            has_value_ = other.has_value_;
            if (has_value_) {
                new (&value) T(other.value);
            }
        }
        return *this;
    }

    Optional& operator=(Optional&& other) {
        if (this != &other) {
            if (has_value_) {
                value.~T();
            }
            has_value_ = other.has_value_;
            if (has_value_) {
                new (&value) T(std::move(other.value));
            }
            other.has_value_ = false;
        }
        return *this;
    }

    bool has_value() const { return has_value_; }
    explicit operator bool() const { return has_value_; }

    T& operator*() { return value; }
    const T& operator*() const { return value; }

    T* operator->() { return &value; }
    const T* operator->() const { return &value; }

    T value_or(const T& default_val) const {
        return has_value_ ? value : default_val;
    }
};

// 解析器类型定义
template <typename T>
using Parser = std::function<Optional<std::pair<T, string_view>>(string_view)>;

// 基础解析器：take - 取固定长度的字符
template <typename T>
Parser<T> take(size_t n) {
    return [n](string_view input) {
        if (input.size >= n) {
            if (std::is_same<T, string_view>::value) {
                T result(input.data, n);
                return Optional<std::pair<T, string_view>>(std::make_pair(result, input.substr(n, input.size - n)));
            } else if (std::is_same<T, std::vector<char>>::value) {
                T result(input.data, input.data + n);
                return Optional<std::pair<T, string_view>>(std::make_pair(result, input.substr(n, input.size - n)));
            }
        }
        return Optional<std::pair<T, string_view>>();
    };
}

// 基础解析器：take_until - 取字符直到遇到指定字符
template <typename T>
Parser<T> take_until(char c) {
    return [c](string_view input) {
        size_t i = 0;
        while (i < input.size && input.data[i] != c) {
            ++i;
        }
        if (i > 0) {
            if (std::is_same<T, string_view>::value) {
                T result(input.data, i);
                return Optional<std::pair<T, string_view>>(std::make_pair(result, input.substr(i, input.size - i)));
            } else if (std::is_same<T, std::vector<char>>::value) {
                T result(input.data, input.data + i);
                return Optional<std::pair<T, string_view>>(std::make_pair(result, input.substr(i, input.size - i)));
            }
        }
        return Optional<std::pair<T, string_view>>();
    };
}

// 基础解析器：tag - 匹配指定的字符串
template <typename T>
Parser<T> tag(const string_view& expected) {
    return [expected](string_view input) {
        if (input.size >= expected.size && input.substr(0, expected.size) == expected) {
            if (std::is_same<T, string_view>::value) {
                T result(expected.data, expected.size);
                return Optional<std::pair<T, string_view>>(std::make_pair(result, input.substr(expected.size, input.size - expected.size)));
            } else if (std::is_same<T, std::vector<char>>::value) {
                T result(expected.data, expected.data + expected.size);
                return Optional<std::pair<T, string_view>>(std::make_pair(result, input.substr(expected.size, input.size - expected.size)));
            }
        }
        return Optional<std::pair<T, string_view>>();
    };
}

// 基础解析器：le_u16 - 解析小端序的uint16_t
template <typename T>
Parser<T> le_u16() {
    static_assert(std::is_same<T, uint16_t>::value, "le_u16 only supports uint16_t");
    
    return [](string_view input) {
        if (input.size >= 2) {
            uint16_t value = static_cast<uint16_t>(input.data[0]) | (static_cast<uint16_t>(input.data[1]) << 8);
            return Optional<std::pair<T, string_view>>(std::make_pair(value, input.substr(2, input.size - 2)));
        }
        return Optional<std::pair<T, string_view>>();
    };
}

// 组合器：seq - 按顺序应用两个解析器
template <typename A, typename B>
Parser<std::pair<A, B>> seq(const Parser<A>& a_parser, const Parser<B>& b_parser) {
    return [a_parser = a_parser, b_parser = b_parser](string_view input) -> Optional<std::pair<std::pair<A, B>, string_view>> {
        auto a_result = a_parser(input);
        if (a_result) {
            auto& [a_value, a_remaining] = *a_result;
            auto b_result = b_parser(a_remaining);
            if (b_result) {
                auto& [b_value, b_remaining] = *b_result;
                return Optional<std::pair<std::pair<A, B>, string_view>>(std::make_pair(std::make_pair(a_value, b_value), b_remaining));
            }
        }
        return Optional<std::pair<std::pair<A, B>, string_view>>();
    };
}

// 组合器：alt - 尝试第一个解析器，如果失败则尝试第二个
template <typename T>
Parser<T> alt(const Parser<T>& parser1, const Parser<T>& parser2) {
    return [parser1 = parser1, parser2 = parser2](string_view input) -> Optional<std::pair<T, string_view>> {
        auto result = parser1(input);
        if (result) {
            return result;
        }
        return parser2(input);
    };
}

// 组合器：many0 - 零次或多次应用解析器，返回结果向量
template <typename T>
Parser<std::vector<T>> many0(const Parser<T>& parser) {
    return [parser = parser](string_view input) -> Optional<std::pair<std::vector<T>, string_view>> {
        std::vector<T> results;
        string_view remaining = input;

        while (true) {
            auto result = parser(remaining);
            if (result) {
                auto& [value, new_remaining] = *result;
                results.push_back(value);
                remaining = new_remaining;
            } else {
                break;
            }
        }

        return Optional<std::pair<std::vector<T>, string_view>>(std::make_pair(results, remaining));
    };
}

// 组合器：many1 - 一次或多次应用解析器，返回结果向量
template <typename T>
Parser<std::vector<T>> many1(const Parser<T>& parser) {
    return [parser = parser](string_view input) -> Optional<std::pair<std::vector<T>, string_view>> {
        auto first_result = parser(input);
        if (!first_result) {
            return Optional<std::pair<std::vector<T>, string_view>>();
        }

        std::vector<T> results;
        auto& [first_value, first_remaining] = *first_result;
        results.push_back(first_value);
        string_view remaining = first_remaining;

        while (true) {
            auto result = parser(remaining);
            if (result) {
                auto& [value, new_remaining] = *result;
                results.push_back(value);
                remaining = new_remaining;
            } else {
                break;
            }
        }

        return Optional<std::pair<std::vector<T>, string_view>>(std::make_pair(results, remaining));
    };
}

// 组合器：separated_list - 解析由分隔符分隔的列表
template <typename T, typename S>
Parser<std::vector<T>> separated_list(const Parser<T>& item_parser, const Parser<S>& separator_parser) {
    return [item_parser = item_parser, separator_parser = separator_parser](string_view input) -> Optional<std::pair<std::vector<T>, string_view>> {
        auto first_result = item_parser(input);
        if (!first_result) {
            // 没有元素，返回空列表
            return Optional<std::pair<std::vector<T>, string_view>>(std::make_pair(std::vector<T>(), input));
        }

        std::vector<T> results;
        auto& [first_value, first_remaining] = *first_result;
        results.push_back(first_value);
        string_view remaining = first_remaining;

        while (true) {
            // 尝试解析分隔符
            auto separator_result = separator_parser(remaining);
            if (!separator_result) {
                break;
            }

            // 分隔符解析成功，尝试解析下一个元素
            remaining = separator_result->second;
            auto item_result = item_parser(remaining);
            if (!item_result) {
                // 分隔符后没有元素，解析失败
                return Optional<std::pair<std::vector<T>, string_view>>();
            }

            // 元素解析成功，添加到结果列表
            results.push_back(item_result->first);
            remaining = item_result->second;
        }

        return Optional<std::pair<std::vector<T>, string_view>>(std::make_pair(results, remaining));
    };
}

// 组合器：delimited - 解析由开始和结束标记包围的内容
template <typename O, typename T, typename C>
Parser<T> delimited(const Parser<O>& open_parser, const Parser<T>& content_parser, const Parser<C>& close_parser) {
    return [open_parser = open_parser, content_parser = content_parser, close_parser = close_parser](string_view input) -> Optional<std::pair<T, string_view>> {
        // 解析开始标记
        auto open_result = open_parser(input);
        if (!open_result) {
            return Optional<std::pair<T, string_view>>();
        }

        // 解析内容
        string_view remaining_after_open = open_result->second;
        auto content_result = content_parser(remaining_after_open);
        if (!content_result) {
            return Optional<std::pair<T, string_view>>();
        }

        // 解析结束标记
        string_view remaining_after_content = content_result->second;
        auto close_result = close_parser(remaining_after_content);
        if (!close_result) {
            return Optional<std::pair<T, string_view>>();
        }

        // 所有部分都解析成功，返回内容
        return Optional<std::pair<T, string_view>>(std::make_pair(content_result->first, close_result->second));
    };
}

// 组合器：preceded - 解析前面带有标记的内容
template <typename O, typename T>
Parser<T> preceded(const Parser<O>& open_parser, const Parser<T>& content_parser) {
    return [open_parser = open_parser, content_parser = content_parser](string_view input) -> Optional<std::pair<T, string_view>> {
        // 解析前面的标记
        auto open_result = open_parser(input);
        if (!open_result) {
            return Optional<std::pair<T, string_view>>();
        }

        // 解析内容
        string_view remaining_after_open = open_result->second;
        auto content_result = content_parser(remaining_after_open);
        if (!content_result) {
            return Optional<std::pair<T, string_view>>();
        }

        // 解析成功，返回内容
        return Optional<std::pair<T, string_view>>(std::make_pair(content_result->first, content_result->second));
    };
}

// 组合器：terminated - 解析后面带有标记的内容
template <typename T, typename C>
Parser<T> terminated(const Parser<T>& content_parser, const Parser<C>& close_parser) {
    return [content_parser = content_parser, close_parser = close_parser](string_view input) -> Optional<std::pair<T, string_view>> {
        // 解析内容
        auto content_result = content_parser(input);
        if (!content_result) {
            return Optional<std::pair<T, string_view>>();
        }

        // 解析后面的标记
        string_view remaining_after_content = content_result->second;
        auto close_result = close_parser(remaining_after_content);
        if (!close_result) {
            return Optional<std::pair<T, string_view>>();
        }

        // 解析成功，返回内容
        return Optional<std::pair<T, string_view>>(std::make_pair(content_result->first, close_result->second));
    };
}

// 组合器：map - 转换解析器的结果
template <typename A, typename B>
Parser<B> map(const Parser<A>& parser, std::function<B(A)>&& transform) {
    return [parser = parser, transform = std::move(transform)](string_view input) -> Optional<std::pair<B, string_view>> {
        auto result = parser(input);
        if (result) {
            auto& [value, remaining] = *result;
            B transformed_value = transform(value);
            return Optional<std::pair<B, string_view>>(std::make_pair(transformed_value, remaining));
        }
        return Optional<std::pair<B, string_view>>();
    };
}

// 组合器：flat_map - 扁平转换解析器结果
template <typename A, typename B>
Parser<B> flat_map(const Parser<A>& parser, std::function<Parser<B>(A)> transform) {
    return [parser = parser, transform = transform](string_view input) -> Optional<std::pair<B, string_view>> {
        auto result = parser(input);
        if (result) {
            auto& [value, remaining] = *result;
            auto next_parser = transform(value);
            return next_parser(remaining);
        }
        return Optional<std::pair<B, string_view>>();
    };
}

// 辅助函数：打印解析结果
template <typename T>
void print_result(const Optional<std::pair<T, string_view>>& result) {
    if (result) {
        std::cout << "解析成功! 结果: ";
        
        // 简单的类型推导打印
        if (std::is_same<T, string_view>::value) {
            std::cout << "\"" << result->first << "\"";
        } else if (std::is_same<T, char>::value) {
            std::cout << "'" << result->first << "'";
        } else if (std::is_same<T, uint16_t>::value) {
            std::cout << result->first;
        } else if (std::is_same<T, std::vector<char>>::value) {
            std::cout << "[ ";
            for (size_t i = 0; i < result->first.size(); ++i) {
                std::cout << "'" << result->first[i] << "' ";
            }
            std::cout << "]";
        } else if (std::is_same<T, std::vector<uint32_t>>::value) {
            std::cout << "[ ";
            for (size_t i = 0; i < result->first.size(); ++i) {
                std::cout << result->first[i] << " ";
            }
            std::cout << "]";
        }
        
        std::cout << ", 剩余输入: \"" << result->second << "\"" << std::endl;
    } else {
        std::cout << "解析失败!" << std::endl;
    }
}

int main() {
    // 示例1: 使用take解析固定长度的字符串
    std::cout << "示例1: take<string_view>(5)" << std::endl;
    auto take_parser = take<string_view>(5);
    auto result1 = take_parser("Hello, World!");
    print_result(result1);
    std::cout << std::endl;

    // 示例2: 使用tag匹配指定的字符串
    std::cout << "示例2: tag<string_view>(\"Hello\")" << std::endl;
    auto tag_parser = tag<string_view>("Hello");
    auto result2 = tag_parser("Hello, World!");
    print_result(result2);
    std::cout << std::endl;

    // 示例3: 使用seq组合两个解析器
    std::cout << "示例3: seq(take<string_view>(5), tag<string_view>(\", \"))" << std::endl;
    auto seq_parser = seq(take<string_view>(5), tag<string_view>(", "));
    auto result3 = seq_parser("Hello, World!");
    if (result3) {
        std::cout << "解析成功! 结果: (\"" << result3->first.first << "\", \"" << result3->first.second << "\"), 剩余输入: \"" << result3->second << "\"" << std::endl;
    } else {
        std::cout << "解析失败!" << std::endl;
    }
    std::cout << std::endl;

    // 示例4: 使用alt尝试两个解析器
    std::cout << "示例4: alt(tag<string_view>(\"Hello\"), tag<string_view>(\"Hi\"))" << std::endl;
    auto alt_parser = alt(tag<string_view>("Hello"), tag<string_view>("Hi"));
    auto result4 = alt_parser("Hi, World!");
    print_result(result4);
    std::cout << std::endl;

    // 示例5: 使用many0零次或多次应用解析器
    std::cout << "示例5: many0(tag<string_view>(\"ab\"))" << std::endl;
    auto many0_parser = many0(tag<string_view>("ab"));
    auto result5 = many0_parser("abababxyz");
    if (result5) {
        std::cout << "解析成功! 结果: [ ";
        for (size_t i = 0; i < result5->first.size(); ++i) {
            std::cout << "\"" << result5->first[i] << "\" ";
        }
        std::cout << "], 剩余输入: \"" << result5->second << "\"" << std::endl;
    } else {
        std::cout << "解析失败!" << std::endl;
    }
    std::cout << std::endl;

    // 示例6: 使用separated_list解析由分隔符分隔的列表
    std::cout << "示例6: separated_list(tag<string_view>(\"a\"), tag<string_view>(\",\"))" << std::endl;
    auto separated_list_parser = separated_list(tag<string_view>("a"), tag<string_view>(","));
    auto result6 = separated_list_parser("a,a,a,xyz");
    if (result6) {
        std::cout << "解析成功! 结果: [ ";
        for (size_t i = 0; i < result6->first.size(); ++i) {
            std::cout << "\"" << result6->first[i] << "\" ";
        }
        std::cout << "], 剩余输入: \"" << result6->second << "\"" << std::endl;
    } else {
        std::cout << "解析失败!" << std::endl;
    }
    std::cout << std::endl;

    // 示例7: 使用delimited解析由开始和结束标记包围的内容
    std::cout << "示例7: delimited(tag<string_view>(\"(\"), take_until<string_view>(')'), tag<string_view>(\")\"))" << std::endl;
    auto delimited_parser = delimited(tag<string_view>("("), take_until<string_view>(')'), tag<string_view>(")"));
    auto result7 = delimited_parser("(Hello, World!)xyz");
    print_result(result7);
    std::cout << std::endl;

    // 示例8: 使用map转换解析器的结果
    std::cout << "示例8: map(take<string_view>(5), [](string_view s) { return std::string(s.data, s.size); })" << std::endl;
    auto map_parser = map(take<string_view>(5), [](string_view s) { return std::string(s.data, s.size); });
    auto result8 = map_parser("Hello, World!");
    if (result8) {
        std::cout << "解析成功! 结果: \"" << result8->first << "\", 剩余输入: \"" << result8->second << "\"" << std::endl;
    } else {
        std::cout << "解析失败!" << std::endl;
    }
    std::cout << std::endl;

    return 0;
}