#include <unistd.h>
#include <sys/socket.h>
#include <vector>
#include "http_parser.h"
#include <cerrno>

using namespace std;

extern size_t g_buffer_size;
extern int g_default_http_port;

bool parse_http_request(int client_fd, HttpRequest &req)
{
    vector<char> buffer(g_buffer_size);
    string data;

    while (data.find("\r\n\r\n") == string::npos)
    {
        ssize_t bytes = recv(client_fd,
                             buffer.data(),
                             buffer.size(),
                             0);
        if (bytes > 0)
        {
            data.append(buffer.data(), bytes);

            if (data.size() > 8192)
                return false; // header too large (DoS protection)

            continue;
        }
        if (bytes == 0)
        {
            // Client closed connection
            return false;
        }
        // bytes < 0 → error or timeout
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Header timeout: client connected but sent nothing
            return false;
        }
        // Any other recv error
        return false;
    }

    req.raw_request = data;
    size_t line_end = data.find("\r\n");
    if (line_end == string::npos)
        return false;

    string request_line = data.substr(0, line_end);
    size_t m1 = request_line.find(' ');
    size_t m2 = request_line.find(' ', m1 + 1);
    if (m1 == string::npos || m2 == string::npos)
        return false;

    req.method = request_line.substr(0, m1);
    string uri = request_line.substr(m1 + 1, m2 - m1 - 1);
    req.port = g_default_http_port;

    if (req.method == "CONNECT")
    {
        size_t colon = uri.find(':');
        if (colon == string::npos)
            return false;
        req.host = uri.substr(0, colon);
        req.port = stoi(uri.substr(colon + 1));
        return true;
    }

    if (uri.find("http://") == 0)
    {
        string rest = uri.substr(7);
        size_t slash = rest.find('/');
        req.host = (slash == string::npos) ? rest : rest.substr(0, slash);
        req.path = (slash == string::npos) ? "/" : rest.substr(slash);
    }
    else
    {
        req.path = uri;
        size_t host_pos = data.find("\r\nHost:");
        if (host_pos == string::npos)
            return false;
        size_t start = host_pos + 7;
        size_t end = data.find("\r\n", start);
        req.host = data.substr(start, end - start);
        while (!req.host.empty() && req.host[0] == ' ')
            req.host.erase(0, 1);
    }

    string new_line = req.method + " " + req.path + " HTTP/1.0";
    req.raw_request.replace(0, line_end, new_line);
    return true;
}
