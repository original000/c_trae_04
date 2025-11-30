#pragma once

#include <string>
#include <chrono>
#include "../storage/SqliteStorage.h"
#include "../cache/LRUCache.h"

class ShortenService {
public:
    ShortenService();
    ~ShortenService();

    bool init();
    int createShortLink(const std::string& long_url, const std::string& custom_alias = "", int expire_seconds = 0);
    std::string getLongUrl(const std::string& short_code, const std::string& ip = "", const std::string& user_agent = "");
    bool disableShortLink(int id);
    LinkStats getLinkStats(int id);

public:
    SqliteStorage& getStorage() {
        return storage_;
    }

private:
    std::string generateShortCode(int id);
    int decodeShortCode(const std::string& short_code);

    SqliteStorage storage_;
    LRUCache cache_;
};
