#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <string>
using namespace std;

struct HttpRequest
{
    string method;
    string host;
    string path;
    int port;
    string raw_request;
};

bool parse_http_request(int client_fd, HttpRequest &req);

#endif
