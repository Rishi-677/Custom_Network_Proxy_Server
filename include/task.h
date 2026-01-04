#ifndef TASK_H
#define TASK_H

#include <string>

using namespace std;

struct Task
{
    int client_fd;
    string client_ip;
    int client_port;
};

#endif
