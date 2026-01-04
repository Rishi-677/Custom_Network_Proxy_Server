#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <vector>
#include <cstring>

#include "forwarder.h"
#include <sys/time.h>

using namespace std;

extern size_t g_buffer_size;
extern int g_socket_timeout;

static void set_socket_timeout(int fd)
{
    timeval tv{};
    tv.tv_sec = g_socket_timeout;
    tv.tv_usec = 0;

    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

static bool send_all(int fd, const char *buf, size_t len)
{
    size_t total_sent = 0;

    while (total_sent < len)
    {
        ssize_t n = send(fd,
                         buf + total_sent,
                         len - total_sent,
                         0);
        if (n <= 0)
            return false;
        total_sent += n;
    }

    return true;
}

size_t forward_tcp(int client_fd, const HttpRequest &req)
{
    vector<char> buffer(g_buffer_size);
    size_t total_bytes = 0;

    hostent *host = gethostbyname(req.host.c_str());
    if (!host)
    {
        close(client_fd);
        return 0;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        close(client_fd);
        return 0;
    }

    set_socket_timeout(server_fd);
    set_socket_timeout(client_fd);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(req.port);
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);

    if (connect(server_fd,
                (sockaddr *)&addr,
                sizeof(addr)) < 0)
    {
        close(server_fd);
        close(client_fd);
        return 0;
    }

    if (!send_all(server_fd,
                  req.raw_request.c_str(),
                  req.raw_request.size()))
    {
        close(server_fd);
        close(client_fd);
        return 0;
    }

    while (true)
    {
        ssize_t bytes = recv(server_fd,
                             buffer.data(),
                             buffer.size(),
                             0);
        if (bytes <= 0)
            break;

        if (!send_all(client_fd,
                      buffer.data(),
                      bytes))
            break;

        total_bytes += bytes;
    }

    close(server_fd);
    close(client_fd);
    return total_bytes;
}

size_t tunnel_tcp(int client_fd, const HttpRequest &req)
{
    vector<char> buffer(g_buffer_size);
    size_t total_bytes = 0;

    hostent *host = gethostbyname(req.host.c_str());
    if (!host)
    {
        close(client_fd);
        return 0;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        close(client_fd);
        return 0;
    }

    set_socket_timeout(server_fd);
    set_socket_timeout(client_fd);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(req.port);
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);

    if (connect(server_fd,
                (sockaddr *)&addr,
                sizeof(addr)) < 0)
    {
        close(server_fd);
        close(client_fd);
        return 0;
    }

    const char *resp =
        "HTTP/1.0 200 Connection Established\r\n\r\n";

    if (!send_all(client_fd, resp, strlen(resp)))
    {
        close(server_fd);
        close(client_fd);
        return 0;
    }

    fd_set fds;

    while (true)
    {
        FD_ZERO(&fds);
        FD_SET(client_fd, &fds);
        FD_SET(server_fd, &fds);

        int max_fd = max(client_fd, server_fd) + 1;

        if (select(max_fd, &fds, NULL, NULL, NULL) <= 0)
            break;

        if (FD_ISSET(client_fd, &fds))
        {
            ssize_t n = recv(client_fd,
                             buffer.data(),
                             buffer.size(),
                             0);
            if (n <= 0)
                break;

            if (!send_all(server_fd,
                          buffer.data(),
                          n))
                break;

            total_bytes += n;
        }

        if (FD_ISSET(server_fd, &fds))
        {
            ssize_t n = recv(server_fd,
                             buffer.data(),
                             buffer.size(),
                             0);
            if (n <= 0)
                break;

            if (!send_all(client_fd,
                          buffer.data(),
                          n))
                break;

            total_bytes += n;
        }
    }

    close(server_fd);
    close(client_fd);
    return total_bytes;
}
