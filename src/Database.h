#ifndef DATABASE_H
#define DATABASE_H

#include <unordered_map>
#include <string>
#include <vector>
#include <chrono>

// 支持的数据类型枚举
enum class ValueType {
    String,
    Hash,
    List
};

// 支持的数据类型
struct Value {
    ValueType type;
    union {
        std::string* str_value;
        std::unordered_map<std::string, std::string>* hash_value;
        std::vector<std::string>* list_value;
    } data;

    // 构造函数
    Value() : type(ValueType::String) {
        data.str_value = new std::string();
    }

    Value(const std::string& str) : type(ValueType::String) {
        data.str_value = new std::string(str);
    }

    Value(const std::unordered_map<std::string, std::string>& hash) : type(ValueType::Hash) {
        data.hash_value = new std::unordered_map<std::string, std::string>(hash);
    }

    Value(const std::vector<std::string>& list) : type(ValueType::List) {
        data.list_value = new std::vector<std::string>(list);
    }

    // 拷贝构造函数
    Value(const Value& other) : type(other.type) {
        switch (type) {
            case ValueType::String:
                data.str_value = new std::string(*other.data.str_value);
                break;
            case ValueType::Hash:
                data.hash_value = new std::unordered_map<std::string, std::string>(*other.data.hash_value);
                break;
            case ValueType::List:
                data.list_value = new std::vector<std::string>(*other.data.list_value);
                break;
        }
    }

    // 移动构造函数
    Value(Value&& other) noexcept : type(other.type), data(other.data) {
        other.type = ValueType::String;
        other.data.str_value = new std::string();
    }

    // 拷贝赋值运算符
    Value& operator=(const Value& other) {
        if (this != &other) {
            // 清理旧数据
            clear();

            // 拷贝新数据
            type = other.type;
            switch (type) {
                case ValueType::String:
                    data.str_value = new std::string(*other.data.str_value);
                    break;
                case ValueType::Hash:
                    data.hash_value = new std::unordered_map<std::string, std::string>(*other.data.hash_value);
                    break;
                case ValueType::List:
                    data.list_value = new std::vector<std::string>(*other.data.list_value);
                    break;
            }
        }
        return *this;
    }

    // 移动赋值运算符
    Value& operator=(Value&& other) noexcept {
        if (this != &other) {
            // 清理旧数据
            clear();

            // 移动新数据
            type = other.type;
            data = other.data;

            // 重置other
            other.type = ValueType::String;
            other.data.str_value = new std::string();
        }
        return *this;
    }

    // 析构函数
    ~Value() {
        clear();
    }

    // 清理数据
    void clear() {
        switch (type) {
            case ValueType::String:
                delete data.str_value;
                break;
            case ValueType::Hash:
                delete data.hash_value;
                break;
            case ValueType::List:
                delete data.list_value;
                break;
        }
    }

    // 获取字符串值
    std::string& get_string() {
        if (type != ValueType::String) {
            clear();
            type = ValueType::String;
            data.str_value = new std::string();
        }
        return *data.str_value;
    }

    // 获取哈希值
    std::unordered_map<std::string, std::string>& get_hash() {
        if (type != ValueType::Hash) {
            clear();
            type = ValueType::Hash;
            data.hash_value = new std::unordered_map<std::string, std::string>();
        }
        return *data.hash_value;
    }

    // 获取列表值
    std::vector<std::string>& get_list() {
        if (type != ValueType::List) {
            clear();
            type = ValueType::List;
            data.list_value = new std::vector<std::string>();
        }
        return *data.list_value;
    }
};

// 带过期时间的键值对
struct ExpirableValue {
    Value value;
    std::chrono::steady_clock::time_point expire_time;
    bool has_expire = false;
};

class Database {
public:
    Database() = default;
    ~Database() = default;

    // 字符串操作
    bool set(const std::string& key, const std::string& value, int expire_seconds = 0);
    std::string* get(const std::string& key);

    // 哈希操作
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    std::string* hget(const std::string& key, const std::string& field);

    // 列表操作
    bool lpush(const std::string& key, const std::string& value);
    std::vector<std::string>* lrange(const std::string& key, int start, int end);

    // 通用操作
    bool del(const std::string& key);
    bool exists(const std::string& key);

    // 获取所有键值对（用于AOF持久化）
    const std::unordered_map<std::string, ExpirableValue>& get_all() const;

private:
    // 检查键是否过期
    bool is_expired(const std::string& key);
    // 清理过期键
    void clean_expired();

    std::unordered_map<std::string, ExpirableValue> data_;
};

#endif // DATABASE_H
