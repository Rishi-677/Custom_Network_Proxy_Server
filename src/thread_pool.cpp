#include "thread_pool.h"
#include "client_handler.h"

ThreadPool::ThreadPool(size_t size) : stop(false)
{
    for (size_t i = 0; i < size; ++i)
    {
        workers.emplace_back(&ThreadPool::worker, this);
    }
}

void ThreadPool::worker()
{
    while (true)
    {
        Task task;

        {
            unique_lock<mutex> lock(queue_mutex);
            condition.wait(lock, [this]
                           { return stop || !tasks.empty(); });

            if (stop && tasks.empty())
                return;

            task = tasks.front();
            tasks.pop();
        }

        handle_client(task);
    }
}

void ThreadPool::enqueue(Task task)
{
    {
        unique_lock<mutex> lock(queue_mutex);
        tasks.push(task);
    }
    condition.notify_one();
}

ThreadPool::~ThreadPool()
{
    {
        unique_lock<mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();

    for (thread &worker : workers)
        worker.join();
}
