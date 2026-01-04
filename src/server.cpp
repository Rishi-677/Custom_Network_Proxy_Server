#include "server.h"
#include "thread_pool.h"
#include "task.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <atomic>
#include <iostream>
#include "logger.h"

using namespace std;

static atomic<bool> shutdown_requested(false);
static int listen_fd = -1;

void request_shutdown()
{
    shutdown_requested.store(true);

    if (listen_fd != -1)
    {
        close(listen_fd);
        listen_fd = -1;
    }
}

void start_server(const string &address, int port, int thread_pool_size)
{
    shutdown_requested.store(false);
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("socket");
        return;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(address.c_str());
    addr.sin_port = htons(port);

    if (bind(listen_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(listen_fd);
        listen_fd = -1;
        return;
    }

    if (listen(listen_fd, SOMAXCONN) < 0)
    {
        perror("listen");
        close(listen_fd);
        listen_fd = -1;
        return;
    }

    ThreadPool pool(thread_pool_size);

    cout << "[INFO] Server listening on "
         << address << ":" << port << endl;

    while (!shutdown_requested.load())
    {
        sockaddr_in client{};
        socklen_t len = sizeof(client);

        int client_fd = accept(listen_fd,
                               (sockaddr *)&client,
                               &len);

        if (client_fd < 0)
        {
            if (shutdown_requested.load())
                break; // graceful exit
            continue;  // transient error
        }

        Task task;
        task.client_fd = client_fd;
        task.client_ip = inet_ntoa(client.sin_addr);
        task.client_port = ntohs(client.sin_port);

        pool.enqueue(task);
    }

    log_event("==================================================");
    log_event("SERVER STOP");
    log_event("==================================================");

    if (listen_fd != -1)
        close(listen_fd);

    cout << "[INFO] Server shutdown complete" << endl;
}
