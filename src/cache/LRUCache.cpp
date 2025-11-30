#include "LRUCache.h"

LRUCache::LRUCache(int capacity) : capacity_(capacity) {
}

LRUCache::~LRUCache() {
}

std::string LRUCache::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (cache_map_.find(key) == cache_map_.end()) {
        return "";
    }

    // Move the accessed item to the front of the list
    cache_list_.splice(cache_list_.begin(), cache_list_, cache_map_[key]);

    return cache_map_[key]->second;
}

void LRUCache::put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (cache_map_.find(key) != cache_map_.end()) {
        // Update the existing item
        cache_map_[key]->second = value;
        cache_list_.splice(cache_list_.begin(), cache_list_, cache_map_[key]);
    } else {
        // Add a new item
        if (cache_list_.size() >= capacity_) {
            // Remove the least recently used item
            std::string lru_key = cache_list_.back().first;
            cache_map_.erase(lru_key);
            cache_list_.pop_back();
        }

        cache_list_.emplace_front(key, value);
        cache_map_[key] = cache_list_.begin();
    }
}

void LRUCache::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (cache_map_.find(key) != cache_map_.end()) {
        cache_list_.erase(cache_map_[key]);
        cache_map_.erase(key);
    }
}

void LRUCache::clear() {
    std::lock_guard<std::mutex> lock(mutex_);

    cache_list_.clear();
    cache_map_.clear();
}

int LRUCache::size() const {
    std::lock_guard<std::mutex> lock(mutex_);

    return cache_list_.size();
}

int LRUCache::capacity() const {
    return capacity_;
}
