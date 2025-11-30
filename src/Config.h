#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config {
public:
    Config(const std::string& config_file);
    ~Config() = default;

    int getWorkerId() const;
    int getDatacenterId() const;

private:
    void parseConfig(const std::string& config_file);

    int worker_id_;
    int datacenter_id_;
};

#endif // CONFIG_H
