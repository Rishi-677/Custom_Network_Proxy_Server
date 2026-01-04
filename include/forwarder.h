#ifndef FORWARDER_H
#define FORWARDER_H

#include "http_parser.h"
#include <cstddef>

size_t forward_tcp(int client_fd, const HttpRequest &req);
size_t tunnel_tcp(int client_fd, const HttpRequest &req);

#endif
