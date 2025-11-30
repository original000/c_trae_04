#pragma once

#include <string>
#include <chrono>
#include <vector>

struct AccessLog {
    int link_id;
    std::string ip;
    std::string user_agent;
    std::chrono::system_clock::time_point accessed_at;
};

struct LinkStats {
    int id;
    std::string short_code;
    std::string long_url;
    std::string custom_alias;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point expire_at;
    bool disabled;
    int total_accesses;
    std::vector<AccessLog> recent_accesses;
};
