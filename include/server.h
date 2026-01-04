#ifndef SERVER_H
#define SERVER_H

#include <string>

void start_server(const std::string &address, int port, int thread_pool_size);
void request_shutdown();

#endif
