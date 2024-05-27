// SPDX-License-Identifier: MIT

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
//#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "async_queue.hpp"


class thread_pool {

    std::vector<std::jthread> workers;

    // Note: we can't use std::function because we're putting std::packaged_task in there,
    // and std::packaged_task is only movable, but std::function always tries to copy.
    using task_type = std::move_only_function<void()>;
    async_queue<task_type> tasks;

    void worker_thread(std::stop_token token);

public:

    thread_pool(std::size_t n);

    ~thread_pool();

    template<typename Func, typename... Args>
    std::future<std::invoke_result_t<std::decay_t<Func>,
                                     std::decay_t<Args>...>>
    submit(Func&& func, Args&&... args)
    {
        auto bfunc = std::bind(std::forward<Func>(func),
                               std::forward<Args>(args)...);
        using Ret = decltype(bfunc());

        std::packaged_task<Ret()> task{std::move(bfunc)};
        auto future = task.get_future();

        if (workers.empty())
            task(); // If no worker thread exists, execute it immediately.
        else
            tasks.push(std::move(task));

        return future;
    }

};


#endif
