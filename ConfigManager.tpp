#ifndef CONFIGMANAGER_TPP
#define CONFIGMANAGER_TPP

#include "ConfigManager.h"

// 模板函数实现

template <typename T>
T ConfigManager::get(const std::string& path, const T& default_value) const {
    const json* value = get_json_by_path(path);
    if (value) {
        try {
            return value->get<T>();
        } catch (...) {
            // 类型转换失败，返回默认值
        }
    }
    return default_value;
}

// 特殊化模板函数，处理vector类型
template <typename T>
std::vector<T> ConfigManager::get(const std::string& path, const std::vector<T>& default_value) const {
    const json* value = get_json_by_path(path);
    if (value && value->is_array()) {
        std::vector<T> result;
        for (const auto& item : *value) {
            try {
                result.push_back(item.get<T>());
            } catch (...) {
                // 类型转换失败，跳过该元素
            }
        }
        return result;
    }
    return default_value;
}

#endif // CONFIGMANAGER_TPP