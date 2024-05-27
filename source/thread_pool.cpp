// SPDX-License-Identifier: MIT

#include "thread_pool.hpp"



void
thread_pool::worker_thread(std::stop_token token)
{
    try {
        while (!token.stop_requested())
            tasks.pop()();
    }
    catch (async_queue<task_type>::stop_request& r) {}
}


thread_pool::thread_pool(std::size_t n)
{
    for (std::size_t i = 0; i < n; ++i)
        workers.emplace_back([this](std::stop_token token) { worker_thread(token); });
}


thread_pool::~thread_pool()
{
    // This will wake up all threads stuck waiting for more tasks,
    // they will all throw tasks_queue::stop_request{}.
    tasks.stop();
    // The jthread destructor will also notify the stop token.
}
