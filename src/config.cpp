#include "config.h"

#include <fstream>
#include <iostream>
#include <cstdlib>

size_t g_buffer_size = 4096;
int g_default_http_port = 80;
int g_socket_timeout = 5;

using namespace std;

static string trim(const string &s)
{
    size_t b = s.find_first_not_of(" \t");
    size_t e = s.find_last_not_of(" \t");
    if (b == string::npos || e == string::npos)
        return "";
    return s.substr(b, e - b + 1);
}

bool load_config(const string &path, RuntimeConfig &cfg)
{
    ifstream file(path);
    if (!file.is_open())
    {
        cerr << "[ERROR] Could not open config file: " << path << endl;
        return false;
    }

    string line;
    while (getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        size_t eq = line.find('=');
        if (eq == string::npos)
            continue;

        string key = trim(line.substr(0, eq));
        string val = trim(line.substr(eq + 1));

        if (key == "listen_address")
            cfg.listen_address = val;
        else if (key == "listen_port")
            cfg.listen_port = stoi(val);
        else if (key == "thread_pool_size")
            cfg.thread_pool_size = stoi(val);
        else if (key == "buffer_size")
            cfg.buffer_size = stoul(val);
        else if (key == "default_http_port")
            cfg.default_http_port = stoi(val);
        else if (key == "log_file")
            cfg.log_file = val;
        else if (key == "max_log_size")
            cfg.max_log_size = stoul(val);
        else if (key == "blocklist_file")
            cfg.blocklist_file = val;
        else if (key == "socket_timeout")
            cfg.socket_timeout = stoi(val);
        else if (key == "metrics_file")
            cfg.metrics_file = val;
    }

    return true;
}
