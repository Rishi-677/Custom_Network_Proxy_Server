#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <cstddef>

struct RuntimeConfig
{
    std::string listen_address = "0.0.0.0";
    int listen_port = 8080;
    int thread_pool_size = 4;

    size_t buffer_size = 4096;
    int default_http_port = 80;

    std::string log_file = "config/logs/proxy.log";
    size_t max_log_size = 5 * 1024 * 1024;

    std::string blocklist_file = "config/blocked_sites.txt";
    int socket_timeout = 5; // seconds

    std::string metrics_file = "config/metrics.txt";
};

bool load_config(const std::string &path, RuntimeConfig &cfg);

#endif
