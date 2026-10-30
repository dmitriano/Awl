#pragma once

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/experimental/promise.hpp>
#include <boost/asio/experimental/use_promise.hpp>
#include <boost/asio/this_coro.hpp>

#include <utility>

namespace awl
{
    // Returns promise<void(std::exception_ptr, T)>, or promise<void(std::exception_ptr)> for void.
    // Starts eagerly; destruction requests cancellation but does not join the task.
    // Keep the executor and task dependencies alive until completion; serialize handle access.
    template <class T>
    [[nodiscard]] auto spawnTask(const boost::asio::any_io_executor executor,
        boost::asio::awaitable<T> task)
    {
        return boost::asio::co_spawn(executor, std::move(task),
            boost::asio::experimental::use_promise);
    }

    template <class T>
    using SpawnedTask = decltype(spawnTask(std::declval<boost::asio::any_io_executor>(),
        std::declval<boost::asio::awaitable<T>>()));

    // Await only the launch on the current executor; await the returned promise separately.
    template <class T>
    [[nodiscard]] boost::asio::awaitable<SpawnedTask<T>> spawnTask(boost::asio::awaitable<T> task)
    {
        const auto executor = co_await boost::asio::this_coro::executor;
        co_return spawnTask(executor, std::move(task));
    }
}
