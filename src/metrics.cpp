#include "metrics.h"

#include <unordered_map>
#include <mutex>
#include <chrono>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

static unordered_map<string, size_t> host_counts;

static size_t total_requests = 0;
static size_t allowed_requests = 0;
static size_t blocked_requests = 0;
static size_t total_bytes = 0;

static chrono::steady_clock::time_point start_time;
static string metrics_path;

static mutex metrics_mutex;

static void write_metrics_locked()
{
    auto now = chrono::steady_clock::now();
    double elapsed_minutes =
        chrono::duration_cast<chrono::duration<double>>(
            now - start_time)
            .count() /
        60.0;

    double rpm = (elapsed_minutes > 0.0)
                     ? total_requests / elapsed_minutes
                     : 0.0;

    vector<pair<string, size_t>> top(host_counts.begin(),
                                     host_counts.end());

    sort(top.begin(), top.end(),
         [](const auto &a, const auto &b)
         {
             return a.second > b.second;
         });

    ofstream out(metrics_path, ios::out); // overwrite
    if (!out.is_open())
        return;

    out << "total_requests=" << total_requests << "\n";
    out << "allowed_requests=" << allowed_requests << "\n";
    out << "blocked_requests=" << blocked_requests << "\n";
    out << "bytes_transferred=" << total_bytes << "\n";
    out << "requests_per_min=" << rpm << "\n";

    out << "top_hosts=";
    size_t limit = min<size_t>(5, top.size());
    for (size_t i = 0; i < limit; ++i)
    {
        out << top[i].first << "("
            << top[i].second << ") ";
    }
    out << "\n";
}

void init_metrics(const string &metrics_file)
{
    lock_guard<mutex> lock(metrics_mutex);

    metrics_path = metrics_file;

    host_counts.clear();
    total_requests = 0;
    allowed_requests = 0;
    blocked_requests = 0;
    total_bytes = 0;

    start_time = chrono::steady_clock::now();

    write_metrics_locked(); // refresh file
}

void record_allowed(const string &host, size_t bytes)
{
    lock_guard<mutex> lock(metrics_mutex);

    ++total_requests;
    ++allowed_requests;
    total_bytes += bytes;
    ++host_counts[host];

    write_metrics_locked();
}

void record_blocked()
{
    lock_guard<mutex> lock(metrics_mutex);

    ++total_requests;
    ++blocked_requests;

    write_metrics_locked();
}

void stop_metrics()
{
    // nothing to do
}
