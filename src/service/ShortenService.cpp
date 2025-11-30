#include "ShortenService.h"
#include "../utils/Config.h"
#include "../utils/Logger.h"

ShortenService::ShortenService() : cache_(Config::getInstance().getCacheCapacity()) {
}

ShortenService::~ShortenService() {
}

bool ShortenService::init() {
    try {
        storage_.init(Config::getInstance().getDatabasePath());
        LOG_INFO("ShortenService initialized successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Failed to initialize ShortenService: ") + e.what());
        return false;
    }
}

int ShortenService::createShortLink(const std::string& long_url, const std::string& custom_alias, int expire_seconds) {
    try {
        std::string short_code;
        if (!custom_alias.empty()) {
            // Check if custom alias is already used
            ShortLink existing_link = storage_.getShortLinkByCode(custom_alias);
            if (existing_link.id != 0) {
                throw std::runtime_error("Custom alias already exists");
            }
            short_code = custom_alias;
        }

        // Calculate expire time
        auto expire_at = std::chrono::system_clock::time_point::max();
        if (expire_seconds > 0) {
            expire_at = std::chrono::system_clock::now() + std::chrono::seconds(expire_seconds);
        }

        // Create the short link in the database
        int id = storage_.createShortLink(long_url, short_code, custom_alias, expire_at);

        // If no custom alias was provided, generate the short code from the id
        if (custom_alias.empty()) {
            short_code = generateShortCode(id);
            // Update the short link with the generated short code
            // Note: This is a workaround since we can't generate the short code before inserting the row
            // In a real application, you might want to use a separate sequence or pre-generate short codes
            std::string sql = "UPDATE links SET short_code = ? WHERE id = ?";
            sqlite3_stmt* stmt;
            int rc = sqlite3_prepare_v2(getStorage().getDb(), sql.c_str(), -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                throw std::runtime_error(std::string("Failed to prepare update_short_code statement: ") + sqlite3_errmsg(getStorage().getDb()));
            }

            sqlite3_bind_text(stmt, 1, short_code.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, id);

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                throw std::runtime_error(std::string("Failed to update short code: ") + sqlite3_errmsg(getStorage().getDb()));
            }

            sqlite3_finalize(stmt);
        }

        LOG_INFO(std::string("Created short link: ") + short_code + " -> " + long_url);
        return id;
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Failed to create short link: ") + e.what());
        throw;
    }
}

std::string ShortenService::getLongUrl(const std::string& short_code, const std::string& ip, const std::string& user_agent) {
    try {
        // First check the cache
        std::string long_url = cache_.get(short_code);
        if (!long_url.empty()) {
            LOG_DEBUG(std::string("Cache hit for short code: ") + short_code);
            return long_url;
        }

        LOG_DEBUG(std::string("Cache miss for short code: ") + short_code);

        // If not in cache, check the database
        ShortLink link = storage_.getShortLinkByCode(short_code);
        if (link.id == 0) {
            throw std::runtime_error("Short code not found");
        }

        // Check if the link is disabled
        if (link.disabled) {
            throw std::runtime_error("Short link is disabled");
        }

        // Check if the link has expired
        if (link.expire_at < std::chrono::system_clock::now()) {
            throw std::runtime_error("Short link has expired");
        }

        // Add the access log
        storage_.addAccessLog(link.id, ip, user_agent);

        // Update the cache
        cache_.put(short_code, link.long_url);

        LOG_INFO(std::string("Redirecting short code: ") + short_code + " -> " + link.long_url);
        return link.long_url;
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Failed to get long URL for short code: ") + short_code + ": " + e.what());
        throw;
    }
}

bool ShortenService::disableShortLink(int id) {
    try {
        bool success = storage_.disableShortLink(id);
        if (success) {
            // Remove the link from cache if it exists
            ShortLink link = storage_.getShortLinkById(id);
            if (link.id != 0) {
                cache_.put(link.short_code, ""); // This will effectively remove it from cache
            }
            LOG_INFO(std::string("Disabled short link with id: ") + std::to_string(id));
        } else {
            LOG_WARNING(std::string("Failed to disable short link with id: ") + std::to_string(id) + " (link not found)");
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Failed to disable short link with id: ") + std::to_string(id) + ": " + e.what());
        throw;
    }
}

LinkStats ShortenService::getLinkStats(int id) {
    try {
        LinkStats stats = storage_.getLinkStats(id);
        if (stats.id == 0) {
            throw std::runtime_error("Link not found");
        }
        LOG_INFO(std::string("Retrieved stats for short link with id: ") + std::to_string(id));
        return stats;
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Failed to get stats for short link with id: ") + std::to_string(id) + ": " + e.what());
        throw;
    }
}

std::string ShortenService::generateShortCode(int id) {
    const std::string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int base = chars.size();
    std::string short_code;

    if (id == 0) {
        return std::string(1, chars[0]);
    }

    while (id > 0) {
        short_code = chars[id % base] + short_code;
        id = id / base;
    }

    // Pad with leading zeros to make it at least 6 characters long
    while (short_code.size() < 6) {
        short_code = chars[0] + short_code;
    }

    return short_code;
}

int ShortenService::decodeShortCode(const std::string& short_code) {
    const std::string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int base = chars.size();
    int id = 0;

    for (char c : short_code) {
        size_t pos = chars.find(c);
        if (pos == std::string::npos) {
            throw std::runtime_error("Invalid character in short code");
        }
        id = id * base + pos;
    }

    return id;
}
