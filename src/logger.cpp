#include "logger.h"
#include <fstream>
#include <mutex>
#include <ctime>
#include <iostream>
#include <sys/stat.h>
#include <cstdio>

using namespace std;

static ofstream log_file;
static mutex log_mutex;
static string log_filename;

// Default: 5 MB
static size_t MAX_LOG_SIZE = 5 * 1024 * 1024;

static string current_timestamp()
{
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf),
             "%Y-%m-%d %H:%M:%S",
             localtime(&now));
    return string(buf);
}

static size_t file_size(const string &path)
{
    struct stat st{};
    if (stat(path.c_str(), &st) == 0)
        return st.st_size;
    return 0;
}

static void rotate_log_if_needed()
{
    if (file_size(log_filename) < MAX_LOG_SIZE)
        return;

    log_file.close();

    string rotated = log_filename + ".1";
    remove(rotated.c_str()); // overwrite old rotation
    rename(log_filename.c_str(), rotated.c_str());

    log_file.open(log_filename, ios::app);
}

void set_log_max_size(size_t bytes)
{
    MAX_LOG_SIZE = bytes;
}

void init_logger(const string &filename)
{
    log_filename = filename;
    log_file.open(log_filename, ios::out | ios::app);

    if (!log_file.is_open())
    {
        cerr << "[ERROR] Cannot open log file: "
             << filename << endl;
        exit(1);
    }
}

void log_event(const string &message)
{
    lock_guard<mutex> lock(log_mutex);

    rotate_log_if_needed();

    log_file << "[" << current_timestamp() << "] "
             << message << endl;
}
