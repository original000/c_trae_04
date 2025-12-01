#include "JsonParser.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <algorithm>

// JsonValue类实现
JsonValue::JsonValue() : type(Null) {
    data.boolValue = false;
}

JsonValue::JsonValue(std::nullptr_t) : type(Null) {
    data.boolValue = false;
}

JsonValue::JsonValue(bool b) : type(Bool) {
    data.boolValue = b;
}

JsonValue::JsonValue(int i) : type(Int) {
    data.intValue = i;
}

JsonValue::JsonValue(double d) : type(Double) {
    data.doubleValue = d;
}

JsonValue::JsonValue(const std::string& s) : type(String) {
    data.stringValue = new std::string(s);
}

JsonValue::JsonValue(const char* s) : type(String) {
    data.stringValue = new std::string(s);
}

JsonValue::JsonValue(const std::vector<JsonValue>& v) : type(Array) {
    data.arrayValue = new std::vector<JsonValue>(v);
}

JsonValue::JsonValue(const std::unordered_map<std::string, JsonValue>& m) : type(Object) {
    data.objectValue = new std::unordered_map<std::string, JsonValue>(m);
}

JsonValue::JsonValue(const JsonValue& other) : type(other.type) {
    switch (type) {
        case Null:
            data.boolValue = false;
            break;
        case Bool:
            data.boolValue = other.data.boolValue;
            break;
        case Int:
            data.intValue = other.data.intValue;
            break;
        case Double:
            data.doubleValue = other.data.doubleValue;
            break;
        case String:
            data.stringValue = new std::string(*other.data.stringValue);
            break;
        case Array:
            data.arrayValue = new std::vector<JsonValue>(*other.data.arrayValue);
            break;
        case Object:
            data.objectValue = new std::unordered_map<std::string, JsonValue>(*other.data.objectValue);
            break;
    }
}

JsonValue& JsonValue::operator=(const JsonValue& other) {
    if (this != &other) {
        // 清理旧数据
        switch (type) {
            case String:
                delete data.stringValue;
                break;
            case Array:
                delete data.arrayValue;
                break;
            case Object:
                delete data.objectValue;
                break;
            default:
                break;
        }

        // 复制新数据
        type = other.type;
        switch (type) {
            case Null:
                data.boolValue = false;
                break;
            case Bool:
                data.boolValue = other.data.boolValue;
                break;
            case Int:
                data.intValue = other.data.intValue;
                break;
            case Double:
                data.doubleValue = other.data.doubleValue;
                break;
            case String:
                data.stringValue = new std::string(*other.data.stringValue);
                break;
            case Array:
                data.arrayValue = new std::vector<JsonValue>(*other.data.arrayValue);
                break;
            case Object:
                data.objectValue = new std::unordered_map<std::string, JsonValue>(*other.data.objectValue);
                break;
        }
    }
    return *this;
}

JsonValue::~JsonValue() {
    switch (type) {
        case String:
            delete data.stringValue;
            break;
        case Array:
            delete data.arrayValue;
            break;
        case Object:
            delete data.objectValue;
            break;
        default:
            break;
    }
}

// 类型检查
bool JsonValue::isNull() const { return type == Null; }
bool JsonValue::isBool() const { return type == Bool; }
bool JsonValue::isInt() const { return type == Int; }
bool JsonValue::isDouble() const { return type == Double; }
bool JsonValue::isString() const { return type == String; }
bool JsonValue::isArray() const { return type == Array; }
bool JsonValue::isObject() const { return type == Object; }

// 类型转换
bool JsonValue::asBool() const { return data.boolValue; }
int JsonValue::asInt() const { return data.intValue; }
double JsonValue::asDouble() const { return data.doubleValue; }
const std::string& JsonValue::asString() const { return *data.stringValue; }
const std::vector<JsonValue>& JsonValue::asArray() const { return *data.arrayValue; }
const std::unordered_map<std::string, JsonValue>& JsonValue::asObject() const { return *data.objectValue; }

// 对象访问
const JsonValue& JsonValue::operator[](const std::string& key) const {
    static JsonValue nullValue;
    if (isObject()) {
        const auto& obj = asObject();
        auto it = obj.find(key);
        if (it != obj.end()) {
            return it->second;
        }
    }
    return nullValue;
}

// 数组访问
const JsonValue& JsonValue::operator[](size_t index) const {
    static JsonValue nullValue;
    if (isArray()) {
        const auto& arr = asArray();
        if (index < arr.size()) {
            return arr[index];
        }
    }
    return nullValue;
}

// 数组大小
size_t JsonValue::size() const {
    if (isArray()) {
        return asArray().size();
    } else if (isObject()) {
        return asObject().size();
    }
    return 0;
}

// 从字符串解析JSON
JsonValue JsonParser::parse(const std::string& json) {
    const char* ptr = json.c_str();
    skipWhitespace(ptr);
    JsonValue value = parseValue(ptr);
    skipWhitespace(ptr);
    if (*ptr != '\0') {
        throw std::runtime_error("Unexpected characters after JSON value");
    }
    return value;
}

// 从文件解析JSON
JsonValue JsonParser::parseFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return parse(buffer.str());
}

// 跳过空白字符
void JsonParser::skipWhitespace(const char*& ptr) {
    while (*ptr != '\0' && std::isspace(static_cast<unsigned char>(*ptr))) {
        ptr++;
    }
}

// 解析值
JsonValue JsonParser::parseValue(const char*& ptr) {
    skipWhitespace(ptr);
    switch (*ptr) {
        case '"':
            return parseString(ptr);
        case '[':
            return parseArray(ptr);
        case '{':
            return parseObject(ptr);
        case 't':
            if (ptr[1] == 'r' && ptr[2] == 'u' && ptr[3] == 'e') {
                ptr += 4;
                return true;
            }
            throw std::runtime_error("Invalid JSON value");
        case 'f':
            if (ptr[1] == 'a' && ptr[2] == 'l' && ptr[3] == 's' && ptr[4] == 'e') {
                ptr += 5;
                return false;
            }
            throw std::runtime_error("Invalid JSON value");
        case 'n':
            if (ptr[1] == 'u' && ptr[2] == 'l' && ptr[3] == 'l') {
                ptr += 4;
                return nullptr;
            }
            throw std::runtime_error("Invalid JSON value");
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return parseNumber(ptr);
        default:
            throw std::runtime_error("Invalid JSON value");
    }
}

// 解析字符串
std::string JsonParser::parseString(const char*& ptr) {
    std::string str;
    ptr++; // 跳过 '"'
    while (*ptr != '\0') {
        if (*ptr == '"') {
            ptr++;
            return str;
        } else if (*ptr == '\\') {
            ptr++;
            switch (*ptr) {
                case '"': str += '"'; break;
                case '\\': str += '\\'; break;
                case '/': str += '/'; break;
                case 'b': str += '\b'; break;
                case 'f': str += '\f'; break;
                case 'n': str += '\n'; break;
                case 'r': str += '\r'; break;
                case 't': str += '\t'; break;
                default: throw std::runtime_error("Invalid JSON escape sequence");
            }
            ptr++;
        } else {
            str += *ptr;
            ptr++;
        }
    }
    throw std::runtime_error("Unterminated JSON string");
}

// 解析数字
JsonValue JsonParser::parseNumber(const char*& ptr) {
    const char* start = ptr;
    if (*ptr == '-') {
        ptr++;
    }
    if (*ptr == '0') {
        ptr++;
        if (std::isdigit(static_cast<unsigned char>(*ptr))) {
            throw std::runtime_error("Invalid JSON number: leading zero");
        }
    } else if (std::isdigit(static_cast<unsigned char>(*ptr))) {
        ptr++;
        while (std::isdigit(static_cast<unsigned char>(*ptr))) {
            ptr++;
        }
    } else {
        throw std::runtime_error("Invalid JSON number");
    }
    bool hasDecimal = false;
    if (*ptr == '.') {
        hasDecimal = true;
        ptr++;
        if (!std::isdigit(static_cast<unsigned char>(*ptr))) {
            throw std::runtime_error("Invalid JSON number: decimal point without fraction");
        }
        ptr++;
        while (std::isdigit(static_cast<unsigned char>(*ptr))) {
            ptr++;
        }
    }
    bool hasExponent = false;
    if (*ptr == 'e' || *ptr == 'E') {
        hasExponent = true;
        ptr++;
        if (*ptr == '+' || *ptr == '-') {
            ptr++;
        }
        if (!std::isdigit(static_cast<unsigned char>(*ptr))) {
            throw std::runtime_error("Invalid JSON number: exponent without digits");
        }
        ptr++;
        while (std::isdigit(static_cast<unsigned char>(*ptr))) {
            ptr++;
        }
    }
    std::string numStr(start, ptr - start);
    if (hasDecimal || hasExponent) {
        return std::stod(numStr);
    } else {
        return std::stoi(numStr);
    }
}

// 解析数组
std::vector<JsonValue> JsonParser::parseArray(const char*& ptr) {
    std::vector<JsonValue> arr;
    ptr++;
    skipWhitespace(ptr);
    if (*ptr == ']') {
        ptr++;
        return arr;
    }
    while (true) {
        arr.push_back(parseValue(ptr));
        skipWhitespace(ptr);
        if (*ptr == ']') {
            ptr++;
            return arr;
        } else if (*ptr == ',') {
            ptr++;
            skipWhitespace(ptr);
        } else {
            throw std::runtime_error("Invalid JSON array: expected ']' or ','");
        }
    }
}

// 解析对象
std::unordered_map<std::string, JsonValue> JsonParser::parseObject(const char*& ptr) {
    std::unordered_map<std::string, JsonValue> obj;
    ptr++;
    skipWhitespace(ptr);
    if (*ptr == '}') {
        ptr++;
        return obj;
    }
    while (true) {
        skipWhitespace(ptr);
        if (*ptr != '"') {
            throw std::runtime_error("Invalid JSON object: expected string key");
        }
        std::string key = parseString(ptr);
        skipWhitespace(ptr);
        if (*ptr != ':') {
            throw std::runtime_error("Invalid JSON object: expected ':' after key");
        }
        ptr++;
        JsonValue value = parseValue(ptr);
        obj[key] = value;
        skipWhitespace(ptr);
        if (*ptr == '}') {
            ptr++;
            return obj;
        } else if (*ptr == ',') {
            ptr++;
            skipWhitespace(ptr);
        } else {
            throw std::runtime_error("Invalid JSON object: expected '}' or ','");
        }
    }
}
