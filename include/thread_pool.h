#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "task.h"

using namespace std;

class ThreadPool
{
public:
    explicit ThreadPool(size_t size);
    ~ThreadPool();

    void enqueue(Task task);

private:
    void worker();

    vector<thread> workers;
    queue<Task> tasks;

    mutex queue_mutex;
    condition_variable condition;
    bool stop;
};

#endif
