#include "BoostExtras/SpawnTask.h"

#include "Awl/Testing/UnitTest.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

namespace
{
    namespace asio = boost::asio;

    void runScenario(std::function<asio::awaitable<void>(asio::any_io_executor)> scenario)
    {
        asio::io_context io;
        const auto executor = asio::make_strand(io);
        bool completed = false;
        std::exception_ptr failure;
        asio::co_spawn(executor, scenario(executor), [&](const std::exception_ptr error)
        {
            failure = error;
            completed = true;
        });
        io.run();

        if (failure)
        {
            std::rethrow_exception(failure);
        }

        AWL_ASSERT(completed);
    }

    asio::awaitable<std::unique_ptr<int>> asyncMakeValue(bool& started)
    {
        started = true;
        co_await asio::post(asio::use_awaitable);
        co_return std::make_unique<int>(42);
    }

    asio::awaitable<void> asyncMarkCompleted(bool& completed)
    {
        co_await asio::post(asio::use_awaitable);
        completed = true;
    }

    asio::awaitable<void> asyncFail()
    {
        co_await asio::post(asio::use_awaitable);
        throw std::runtime_error("spawned task failed");
    }

    asio::awaitable<void> asyncWait(asio::steady_timer& timer, bool& finished)
    {
        try
        {
            co_await timer.async_wait(asio::use_awaitable);
        }
        catch (...)
        {
            finished = true;

            throw;
        }
    }
}

AWL_TEST(SpawnTask_EagerExecutionAndMoveOnlyResult)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        bool started = false;
        auto task = awl::spawnTask(executor, asyncMakeValue(started));

        while (!task.completed())
        {
            co_await asio::post(asio::use_awaitable);
        }

        AWL_ASSERT(started);
        const std::unique_ptr<int> value = co_await std::move(task);
        AWL_ASSERT_EQUAL(42, *value);
    });
}

AWL_TEST(SpawnTask_VoidResultAndException)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        bool completed = false;
        auto task = co_await awl::spawnTask(asyncMarkCompleted(completed));
        co_await std::move(task);
        AWL_ASSERT(completed);
        auto failed_task = awl::spawnTask(executor, asyncFail());
        bool caught = false;

        try
        {
            co_await std::move(failed_task);
        }
        catch (const std::runtime_error& error)
        {
            caught = true;
            AWL_ASSERT_EQUAL(std::string("spawned task failed"), std::string(error.what()));
        }

        AWL_ASSERT(caught);
    });
}

AWL_TEST(SpawnTask_CancelAndJoin)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        asio::steady_timer timer(executor, asio::steady_timer::time_point::max());
        bool finished = false;
        auto task = awl::spawnTask(executor, asyncWait(timer, finished));
        co_await asio::post(asio::use_awaitable);
        task.cancel();
        bool cancelled = false;

        try
        {
            co_await std::move(task);
        }
        catch (const boost::system::system_error& error)
        {
            cancelled = error.code() == asio::error::operation_aborted;
        }

        AWL_ASSERT(cancelled && finished && task.completed());
    });
}

AWL_TEST(SpawnTask_DestructorCancelsOnCurrentExecutor)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        asio::steady_timer timer(executor, asio::steady_timer::time_point::max());
        bool finished = false;

        {
            auto task = co_await awl::spawnTask(asyncWait(timer, finished));
            co_await asio::post(asio::use_awaitable);
            AWL_ASSERT(!task.completed());
        }

        while (!finished)
        {
            co_await asio::post(asio::use_awaitable);
        }
    });
}
