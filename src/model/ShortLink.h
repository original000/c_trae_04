#pragma once

#include <string>
#include <chrono>

struct ShortLink {
    int id;
    std::string short_code;
    std::string long_url;
    std::string custom_alias;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point expire_at;
    bool disabled;
};
