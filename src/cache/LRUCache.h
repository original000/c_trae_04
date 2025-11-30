#pragma once

#include <unordered_map>
#include <list>
#include <mutex>
#include <string>

class LRUCache {
public:
    explicit LRUCache(int capacity) : capacity_(capacity) {}

    std::string get(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = map_.find(key);
        if (it == map_.end()) {
            return "";
        }

        // Move the accessed element to the front of the list
        list_.splice(list_.begin(), list_, it->second);

        return it->second->second;
    }

    void put(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = map_.find(key);
        if (it != map_.end()) {
            // Update the existing value and move to front
            it->second->second = value;
            list_.splice(list_.begin(), list_, it->second);
            return;
        }

        // Check if cache is full
        if (list_.size() >= capacity_) {
            // Remove the least recently used element (back of the list)
            auto last = list_.back();
            map_.erase(last.first);
            list_.pop_back();
        }

        // Add the new element to the front of the list
        list_.emplace_front(key, value);
        map_[key] = list_.begin();
    }

private:
    int capacity_;
    std::list<std::pair<std::string, std::string>> list_;
    std::unordered_map<std::string, std::list<std::pair<std::string, std::string>>::iterator> map_;
    std::mutex mutex_;
};
