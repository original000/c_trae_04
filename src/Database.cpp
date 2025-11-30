#include "Database.h"
#include <algorithm>

// 字符串操作
bool Database::set(const std::string& key, const std::string& value, int expire_seconds) {
    clean_expired();

    ExpirableValue ev;
    ev.value.type = ValueType::String;
    ev.value.data.str_value = new std::string(value);

    if (expire_seconds > 0) {
        ev.expire_time = std::chrono::steady_clock::now() + std::chrono::seconds(expire_seconds);
        ev.has_expire = true;
    }

    data_[key] = ev;
    return true;
}

std::string* Database::get(const std::string& key) {
    clean_expired();

    if (!exists(key)) {
        return nullptr;
    }

    // 检查键的类型是否为字符串
    if (data_[key].value.type != ValueType::String) {
        return nullptr;
    }

    // 返回字符串值
    return new std::string(*data_[key].value.data.str_value);
}

// 哈希操作
bool Database::hset(const std::string& key, const std::string& field, const std::string& value) {
    clean_expired();

    if (!exists(key)) {
        // 创建新的哈希
        ExpirableValue ev;
        ev.value.type = ValueType::Hash;
        ev.value.data.hash_value = new std::unordered_map<std::string, std::string>();
        (*ev.value.data.hash_value)[field] = value;
        data_[key] = ev;
    } else {
        // 检查键的类型是否为哈希
        if (data_[key].value.type != ValueType::Hash) {
            return false;
        }
        // 直接获取哈希值并设置字段
        (*data_[key].value.data.hash_value)[field] = value;
    }

    return true;
}

std::string* Database::hget(const std::string& key, const std::string& field) {
    clean_expired();

    if (!exists(key)) {
        return nullptr;
    }

    // 检查键的类型是否为哈希
    if (data_[key].value.type != ValueType::Hash) {
        return nullptr;
    }

    // 直接获取哈希值并查找字段
    const auto& hash = *data_[key].value.data.hash_value;
    auto it = hash.find(field);
    if (it != hash.end()) {
        return new std::string(it->second);
    }

    return nullptr;
}

// 列表操作
bool Database::lpush(const std::string& key, const std::string& value) {
    clean_expired();

    if (!exists(key)) {
        // 创建新的列表
        ExpirableValue ev;
        ev.value.type = ValueType::List;
        ev.value.data.list_value = new std::vector<std::string>();
        ev.value.data.list_value->push_back(value);
        data_[key] = ev;
    } else {
        // 检查键的类型是否为列表
        if (data_[key].value.type != ValueType::List) {
            return false;
        }
        // 直接获取列表值并在开头插入
        auto& list = *data_[key].value.data.list_value;
        list.insert(list.begin(), value);
    }

    return true;
}

std::vector<std::string>* Database::lrange(const std::string& key, int start, int end) {
    clean_expired();

    if (!exists(key)) {
        return nullptr;
    }

    // 检查键的类型是否为列表
    if (data_[key].value.type != ValueType::List) {
        return nullptr;
    }

    // 直接获取列表值
    const auto& list = *data_[key].value.data.list_value;
    int size = list.size();

    // 处理负数索引
    if (start < 0) start = size + start;
    if (end < 0) end = size + end;

    // 边界检查
    if (start < 0) start = 0;
    if (end >= size) end = size - 1;
    if (start > end) return new std::vector<std::string>();

    // 截取子列表
    return new std::vector<std::string>(list.begin() + start, list.begin() + end + 1);
}

// 通用操作
bool Database::del(const std::string& key) {
    clean_expired();
    return data_.erase(key) > 0;
}

bool Database::exists(const std::string& key) {
    auto it = data_.find(key);
    if (it == data_.end()) {
        return false;
    }

    if (it->second.has_expire) {
        if (std::chrono::steady_clock::now() > it->second.expire_time) {
            data_.erase(it);
            return false;
        }
    }

    return true;
}

// 获取所有键值对（用于AOF持久化）
const std::unordered_map<std::string, ExpirableValue>& Database::get_all() const {
    return data_;
}

// 检查键是否过期
bool Database::is_expired(const std::string& key) {
    auto it = data_.find(key);
    if (it == data_.end()) {
        return false;
    }

    if (it->second.has_expire) {
        if (std::chrono::steady_clock::now() > it->second.expire_time) {
            return true;
        }
    }

    return false;
}

// 清理过期键
void Database::clean_expired() {
    for (auto it = data_.begin(); it != data_.end();) {
        if (it->second.has_expire && std::chrono::steady_clock::now() > it->second.expire_time) {
            it = data_.erase(it);
        } else {
            ++it;
        }
    }
}
