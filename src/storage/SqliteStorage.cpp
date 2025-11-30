#include "SqliteStorage.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>

SqliteStorage::SqliteStorage() : db_(nullptr), stmt_create_link_(nullptr), stmt_get_link_by_code_(nullptr), stmt_get_link_by_id_(nullptr), stmt_disable_link_(nullptr), stmt_add_access_log_(nullptr), stmt_get_link_stats_(nullptr), stmt_get_recent_access_logs_(nullptr) {
}

SqliteStorage::~SqliteStorage() {
    // Finalize all statements
    if (stmt_create_link_) sqlite3_finalize(stmt_create_link_);
    if (stmt_get_link_by_code_) sqlite3_finalize(stmt_get_link_by_code_);
    if (stmt_get_link_by_id_) sqlite3_finalize(stmt_get_link_by_id_);
    if (stmt_disable_link_) sqlite3_finalize(stmt_disable_link_);
    if (stmt_add_access_log_) sqlite3_finalize(stmt_add_access_log_);
    if (stmt_get_link_stats_) sqlite3_finalize(stmt_get_link_stats_);
    if (stmt_get_recent_access_logs_) sqlite3_finalize(stmt_get_recent_access_logs_);

    // Close the database
    if (db_) sqlite3_close(db_);
}

bool SqliteStorage::init(const std::string& db_path) {
    // Open the database
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to open database: ") + sqlite3_errmsg(db_));
    }

    // Create tables if they don't exist
    if (!createTables()) {
        return false;
    }

    // Prepare statements
    const char* sql_create_link = "INSERT INTO links (short_code, long_url, custom_alias, created_at, expire_at, disabled) VALUES (?, ?, ?, ?, ?, ?)";
    rc = sqlite3_prepare_v2(db_, sql_create_link, -1, &stmt_create_link_, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare create_link statement: ") + sqlite3_errmsg(db_));
    }

    const char* sql_get_link_by_code = "SELECT id, short_code, long_url, custom_alias, created_at, expire_at, disabled FROM links WHERE short_code = ?";
    rc = sqlite3_prepare_v2(db_, sql_get_link_by_code, -1, &stmt_get_link_by_code_, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare get_link_by_code statement: ") + sqlite3_errmsg(db_));
    }

    const char* sql_get_link_by_id = "SELECT id, short_code, long_url, custom_alias, created_at, expire_at, disabled FROM links WHERE id = ?";
    rc = sqlite3_prepare_v2(db_, sql_get_link_by_id, -1, &stmt_get_link_by_id_, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare get_link_by_id statement: ") + sqlite3_errmsg(db_));
    }

    const char* sql_disable_link = "UPDATE links SET disabled = 1 WHERE id = ?";
    rc = sqlite3_prepare_v2(db_, sql_disable_link, -1, &stmt_disable_link_, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare disable_link statement: ") + sqlite3_errmsg(db_));
    }

    const char* sql_add_access_log = "INSERT INTO access_logs (link_id, ip, user_agent, accessed_at) VALUES (?, ?, ?, ?)";
    rc = sqlite3_prepare_v2(db_, sql_add_access_log, -1, &stmt_add_access_log_, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare add_access_log statement: ") + sqlite3_errmsg(db_));
    }

    const char* sql_get_link_stats = "SELECT links.id, links.short_code, links.long_url, links.custom_alias, links.created_at, links.expire_at, links.disabled, COUNT(access_logs.id) as total_accesses FROM links LEFT JOIN access_logs ON links.id = access_logs.link_id WHERE links.id = ? GROUP BY links.id";
    rc = sqlite3_prepare_v2(db_, sql_get_link_stats, -1, &stmt_get_link_stats_, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare get_link_stats statement: ") + sqlite3_errmsg(db_));
    }

    const char* sql_get_recent_access_logs = "SELECT link_id, ip, user_agent, accessed_at FROM access_logs WHERE link_id = ? ORDER BY accessed_at DESC LIMIT 20";
    rc = sqlite3_prepare_v2(db_, sql_get_recent_access_logs, -1, &stmt_get_recent_access_logs_, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare get_recent_access_logs statement: ") + sqlite3_errmsg(db_));
    }

    return true;
}

int SqliteStorage::createShortLink(const std::string& long_url, const std::string& short_code, const std::string& custom_alias, const std::chrono::system_clock::time_point& expire_at) {
    // Bind parameters
    sqlite3_bind_text(stmt_create_link_, 1, short_code.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_create_link_, 2, long_url.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_create_link_, 3, custom_alias.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_create_link_, 4, timePointToString(std::chrono::system_clock::now()).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_create_link_, 5, timePointToString(expire_at).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt_create_link_, 6, 0);

    // Execute the statement
    int rc = sqlite3_step(stmt_create_link_);
    if (rc != SQLITE_DONE) {
        sqlite3_reset(stmt_create_link_);
        throw std::runtime_error(std::string("Failed to create short link: ") + sqlite3_errmsg(db_));
    }

    // Get the last inserted row ID
    int last_id = sqlite3_last_insert_rowid(db_);

    // Reset the statement for reuse
    sqlite3_reset(stmt_create_link_);

    return last_id;
}

ShortLink SqliteStorage::getShortLinkByCode(const std::string& short_code) {
    ShortLink link = {0, "", "", "", std::chrono::system_clock::now(), std::chrono::system_clock::now(), false};

    // Bind parameters
    sqlite3_bind_text(stmt_get_link_by_code_, 1, short_code.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    int rc = sqlite3_step(stmt_get_link_by_code_);
    if (rc == SQLITE_ROW) {
        // Read the results
        link.id = sqlite3_column_int(stmt_get_link_by_code_, 0);
        link.short_code = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_by_code_, 1));
        link.long_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_by_code_, 2));
        link.custom_alias = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_by_code_, 3));
        link.created_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_by_code_, 4)));
        link.expire_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_by_code_, 5)));
        link.disabled = sqlite3_column_int(stmt_get_link_by_code_, 6) != 0;
    }

    // Reset the statement for reuse
    sqlite3_reset(stmt_get_link_by_code_);

    return link;
}

ShortLink SqliteStorage::getShortLinkById(int id) {
    ShortLink link = {0, "", "", "", std::chrono::system_clock::now(), std::chrono::system_clock::now(), false};

    // Bind parameters
    sqlite3_bind_int(stmt_get_link_by_id_, 1, id);

    // Execute the statement
    int rc = sqlite3_step(stmt_get_link_by_id_);
    if (rc == SQLITE_ROW) {
        // Read the results
        link.id = sqlite3_column_int(stmt_get_link_by_id_, 0);
        link.short_code = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_by_id_, 1));
        link.long_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_by_id_, 2));
        link.custom_alias = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_by_id_, 3));
        link.created_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_by_id_, 4)));
        link.expire_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_by_id_, 5)));
        link.disabled = sqlite3_column_int(stmt_get_link_by_id_, 6) != 0;
    }

    // Reset the statement for reuse
    sqlite3_reset(stmt_get_link_by_id_);

    return link;
}

bool SqliteStorage::disableShortLink(int id) {
    // Bind parameters
    sqlite3_bind_int(stmt_disable_link_, 1, id);

    // Execute the statement
    int rc = sqlite3_step(stmt_disable_link_);
    if (rc != SQLITE_DONE) {
        sqlite3_reset(stmt_disable_link_);
        throw std::runtime_error(std::string("Failed to disable short link: ") + sqlite3_errmsg(db_));
    }

    // Check if any rows were affected
    int rows_affected = sqlite3_changes(db_);

    // Reset the statement for reuse
    sqlite3_reset(stmt_disable_link_);

    return rows_affected > 0;
}

bool SqliteStorage::addAccessLog(int link_id, const std::string& ip, const std::string& user_agent) {
    // Bind parameters
    sqlite3_bind_int(stmt_add_access_log_, 1, link_id);
    sqlite3_bind_text(stmt_add_access_log_, 2, ip.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_add_access_log_, 3, user_agent.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_add_access_log_, 4, timePointToString(std::chrono::system_clock::now()).c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    int rc = sqlite3_step(stmt_add_access_log_);
    if (rc != SQLITE_DONE) {
        sqlite3_reset(stmt_add_access_log_);
        throw std::runtime_error(std::string("Failed to add access log: ") + sqlite3_errmsg(db_));
    }

    // Reset the statement for reuse
    sqlite3_reset(stmt_add_access_log_);

    return true;
}

LinkStats SqliteStorage::getLinkStats(int id) {
    LinkStats stats = {0, "", "", "", std::chrono::system_clock::now(), std::chrono::system_clock::now(), false, 0, {}};

    // Bind parameters for link stats
    sqlite3_bind_int(stmt_get_link_stats_, 1, id);

    // Execute the statement for link stats
    int rc = sqlite3_step(stmt_get_link_stats_);
    if (rc == SQLITE_ROW) {
        // Read the results
        stats.id = sqlite3_column_int(stmt_get_link_stats_, 0);
        stats.short_code = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_stats_, 1));
        stats.long_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_stats_, 2));
        stats.custom_alias = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_stats_, 3));
        stats.created_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_stats_, 4)));
        stats.expire_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_link_stats_, 5)));
        stats.disabled = sqlite3_column_int(stmt_get_link_stats_, 6) != 0;
        stats.total_accesses = sqlite3_column_int(stmt_get_link_stats_, 7);
    }

    // Reset the statement for reuse
    sqlite3_reset(stmt_get_link_stats_);

    // Bind parameters for recent access logs
    sqlite3_bind_int(stmt_get_recent_access_logs_, 1, id);

    // Execute the statement for recent access logs
    while ((rc = sqlite3_step(stmt_get_recent_access_logs_)) == SQLITE_ROW) {
        AccessLog log;
        log.link_id = sqlite3_column_int(stmt_get_recent_access_logs_, 0);
        log.ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_recent_access_logs_, 1));
        log.user_agent = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_recent_access_logs_, 2));
        log.accessed_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_recent_access_logs_, 3)));
        stats.recent_accesses.push_back(log);
    }

    // Reset the statement for reuse
    sqlite3_reset(stmt_get_recent_access_logs_);

    return stats;
}

bool SqliteStorage::createTables() {
    // Create links table
    const char* sql_create_links = 
        "CREATE TABLE IF NOT EXISTS links (" 
        "id INTEGER PRIMARY KEY AUTOINCREMENT," 
        "short_code TEXT UNIQUE NOT NULL," 
        "long_url TEXT NOT NULL," 
        "custom_alias TEXT UNIQUE," 
        "created_at TEXT NOT NULL," 
        "expire_at TEXT NOT NULL," 
        "disabled INTEGER NOT NULL DEFAULT 0" 
        ");";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql_create_links, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string error_msg = std::string("Failed to create links table: ") + err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(error_msg);
    }

    // Create access_logs table
    const char* sql_create_access_logs = 
        "CREATE TABLE IF NOT EXISTS access_logs (" 
        "id INTEGER PRIMARY KEY AUTOINCREMENT," 
        "link_id INTEGER NOT NULL," 
        "ip TEXT NOT NULL," 
        "user_agent TEXT NOT NULL," 
        "accessed_at TEXT NOT NULL," 
        "FOREIGN KEY (link_id) REFERENCES links (id) ON DELETE CASCADE" 
        ");";

    rc = sqlite3_exec(db_, sql_create_access_logs, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string error_msg = std::string("Failed to create access_logs table: ") + err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(error_msg);
    }

    // Create index on access_logs.link_id for faster queries
    const char* sql_create_access_logs_index = 
        "CREATE INDEX IF NOT EXISTS idx_access_logs_link_id ON access_logs (link_id);";

    rc = sqlite3_exec(db_, sql_create_access_logs_index, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string error_msg = std::string("Failed to create access_logs index: ") + err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(error_msg);
    }

    return true;
}

std::string SqliteStorage::timePointToString(const std::chrono::system_clock::time_point& tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) % 1000;

    std::tm tm = *std::localtime(&time_t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}

std::chrono::system_clock::time_point SqliteStorage::stringToTimePoint(const std::string& str) {
    std::tm tm = {};
    std::istringstream iss(str);

    // Parse the time string (format: YYYY-MM-DD HH:MM:SS.fff)
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (iss.fail()) {
        throw std::runtime_error("Failed to parse time string");
    }

    // Parse milliseconds
    int ms = 0;
    if (iss.peek() == '.') {
        iss.ignore();
        std::string ms_str;
        std::getline(iss, ms_str, '.');
        if (ms_str.length() > 3) {
            ms_str = ms_str.substr(0, 3);
        } else if (ms_str.length() < 3) {
            ms_str += std::string(3 - ms_str.length(), '0');
        }
        ms = std::stoi(ms_str);
    }

    // Convert to time_point
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    tp += std::chrono::milliseconds(ms);

    return tp;
}
