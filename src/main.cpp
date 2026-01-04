#include <iostream>
#include <csignal>
#include <cstdlib>
#include <cstddef>

#include "server.h"
#include "blocklist.h"
#include "logger.h"
#include "config.h"
#include "metrics.h"

using namespace std;

extern size_t g_buffer_size;
extern int g_default_http_port;
extern int g_socket_timeout;

void handle_signal(int)
{
    cout << "\n[INFO] Caught termination signal" << endl;
    request_shutdown();
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    RuntimeConfig cfg;
    if (!load_config("config/proxy.conf", cfg))
        return 1;

    if (argc == 2)
    {
        int port = atoi(argv[1]);
        if (port <= 0 || port > 65535)
        {
            cerr << "[ERROR] Invalid port number" << endl;
            return 1;
        }
        cfg.listen_port = port;
    }

    g_buffer_size = cfg.buffer_size;
    g_default_http_port = cfg.default_http_port;
    g_socket_timeout = cfg.socket_timeout;

    if (!load_blocklist(cfg.blocklist_file))
        return 1;

    init_logger(cfg.log_file);
    set_log_max_size(cfg.max_log_size);
    init_metrics(cfg.metrics_file);

    log_event("==================================================");
    log_event("SERVER START");
    log_event("==================================================");

    cout << "[INFO] Starting proxy on "
         << cfg.listen_address << ":"
         << cfg.listen_port << endl;

    start_server(cfg.listen_address,
                 cfg.listen_port,
                 cfg.thread_pool_size);

    cout << "[INFO] Proxy stopped cleanly" << endl;
    stop_metrics();
    return 0;
}
