#include "client_handler.h"
#include "http_parser.h"
#include "forwarder.h"
#include "blocklist.h"
#include "logger.h"
#include "metrics.h"
#include "task.h"

#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>

extern int g_socket_timeout;

using namespace std;
void handle_client(const Task &task)
{
    HttpRequest req;
    size_t bytes = 0;

    timeval tv{};
    tv.tv_sec = g_socket_timeout;
    tv.tv_usec = 0;

    setsockopt(task.client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(task.client_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (!parse_http_request(task.client_fd, req))
    {
        close(task.client_fd);
        return;
    }

    string request_line =
        req.method + " " + req.path + " HTTP/1.0";

    string host_port =
        req.host + ":" + to_string(req.port);

    // BLOCKED
    if (is_blocked(req.host))
    {
        record_blocked();
        log_event(
            task.client_ip + ":" + to_string(task.client_port) +
            " | \"" + request_line + "\"" +
            " | " + host_port +
            " | BLOCKED | 403 | bytes=0");

        const char *resp =
            "HTTP/1.0 403 Forbidden\r\n"
            "Content-Length: 0\r\n\r\n";

        send(task.client_fd, resp, strlen(resp), MSG_NOSIGNAL);
        close(task.client_fd);
        return;
    }

    // CONNECT
    if (req.method == "CONNECT")
    {
        bytes = tunnel_tcp(task.client_fd, req);
        record_allowed(req.host, bytes);

        log_event(
            task.client_ip + ":" + to_string(task.client_port) +
            " | \"" + request_line + "\"" +
            " | " + host_port +
            " | ALLOWED | 200 | bytes=" +
            to_string(bytes));
        return;
    }

    // HTTP
    bytes = forward_tcp(task.client_fd, req);
    record_allowed(req.host, bytes);

    log_event(
        task.client_ip + ":" + to_string(task.client_port) +
        " | \"" + request_line + "\"" +
        " | " + host_port +
        " | ALLOWED | 200 | bytes=" +
        to_string(bytes));
}
