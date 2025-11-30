#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include "../model/ShortLink.h"
#include "../model/Stats.h"

class SqliteStorage {
public:
    SqliteStorage();
    ~SqliteStorage();

    bool init(const std::string& db_path);
    int createShortLink(const std::string& long_url, const std::string& short_code, const std::string& custom_alias, const std::chrono::system_clock::time_point& expire_at);
    ShortLink getShortLinkByCode(const std::string& short_code);
    ShortLink getShortLinkById(int id);
    bool disableShortLink(int id);
    bool addAccessLog(int link_id, const std::string& ip, const std::string& user_agent);
    LinkStats getLinkStats(int id);

public:
    sqlite3* getDb() const {
        return db_;
    }

private:
    bool createTables();
    std::chrono::system_clock::time_point stringToTimePoint(const std::string& str);

public:
    std::string timePointToString(const std::chrono::system_clock::time_point& tp);

    sqlite3* db_;
    // Precompiled statements
    sqlite3_stmt* stmt_create_link_;
    sqlite3_stmt* stmt_get_link_by_code_;
    sqlite3_stmt* stmt_get_link_by_id_;
    sqlite3_stmt* stmt_disable_link_;
    sqlite3_stmt* stmt_add_access_log_;
    sqlite3_stmt* stmt_get_link_stats_;
    sqlite3_stmt* stmt_get_recent_access_logs_;
};
