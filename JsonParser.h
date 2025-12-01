#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

// 简单的JSON解析器实现
class JsonValue {
public:
    enum Type {
        Null,
        Bool,
        Int,
        Double,
        String,
        Array,
        Object
    };

    JsonValue();
    JsonValue(std::nullptr_t);
    JsonValue(bool b);
    JsonValue(int i);
    JsonValue(double d);
    JsonValue(const std::string& s);
    JsonValue(const char* s);
    JsonValue(const std::vector<JsonValue>& v);
    JsonValue(const std::unordered_map<std::string, JsonValue>& m);
    JsonValue(const JsonValue& other);
    JsonValue& operator=(const JsonValue& other);
    ~JsonValue();

    // 类型检查
    bool isNull() const;
    bool isBool() const;
    bool isInt() const;
    bool isDouble() const;
    bool isString() const;
    bool isArray() const;
    bool isObject() const;

    // 类型转换
    bool asBool() const;
    int asInt() const;
    double asDouble() const;
    const std::string& asString() const;
    const std::vector<JsonValue>& asArray() const;
    const std::unordered_map<std::string, JsonValue>& asObject() const;

    // 对象访问
    const JsonValue& operator[](const std::string& key) const;

    // 数组访问
    const JsonValue& operator[](size_t index) const;

    // 数组大小
    size_t size() const;

private:
    Type type;
    union {
        bool boolValue;
        int intValue;
        double doubleValue;
        std::string* stringValue;
        std::vector<JsonValue>* arrayValue;
        std::unordered_map<std::string, JsonValue>* objectValue;
    } data;
};

class JsonParser {
public:
    // 从字符串解析JSON
    static JsonValue parse(const std::string& json);

    // 从文件解析JSON
    static JsonValue parseFile(const std::string& filename);

private:
    // 跳过空白字符
    static void skipWhitespace(const char*& ptr);

    // 解析值
    static JsonValue parseValue(const char*& ptr);

    // 解析字符串
    static std::string parseString(const char*& ptr);

    // 解析数字
    static JsonValue parseNumber(const char*& ptr);

    // 解析数组
    static std::vector<JsonValue> parseArray(const char*& ptr);

    // 解析对象
    static std::unordered_map<std::string, JsonValue> parseObject(const char*& ptr);
};

#endif // JSONPARSER_H