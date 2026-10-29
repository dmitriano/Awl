#include "BoostExtras/AsyncMutex.h"

#include "Awl/Testing/UnitTest.h"

#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/this_coro.hpp>

#include <atomic>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

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

    bool isCancellation(const std::exception_ptr failure)
    {
        if (failure)
        {
            try
            {
                std::rethrow_exception(failure);
            }
            catch (const boost::system::system_error& error)
            {
                return error.code() == asio::error::operation_aborted ||
                    error.code() == asio::experimental::error::channel_cancelled;
            }
        }

        return false;
    }
}

AWL_TEST(AsyncMutex_SerializesAcrossSuspension)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::AsyncMutex mutex(executor);
        auto owner = co_await mutex.asyncLock();
        constexpr int count = 12;
        awl::Channel<void(boost::system::error_code, std::exception_ptr)> completed(executor, count);
        int started = 0;
        int active = 0;
        int finished = 0;

        for (int i = 0; i < count; ++i)
        {
            asio::co_spawn(executor, [&]() -> asio::awaitable<void>
            {
                ++started;
                const auto lock = co_await mutex.asyncLock();
                AWL_ASSERT(active++ == 0);
                co_await asio::post(executor, asio::use_awaitable);
                AWL_ASSERT(active == 1);
                --active;
                ++finished;
            }, [&completed](const std::exception_ptr error)
            {
                completed.try_send(boost::system::error_code{}, error);
            });
        }

        while (started != count)
        {
            co_await asio::post(executor, asio::use_awaitable);
        }

        AWL_ASSERT(finished == 0);
        owner.unlock();

        for (int i = 0; i < count; ++i)
        {
            const auto error = co_await completed.async_receive(asio::use_awaitable);

            if (error)
            {
                std::rethrow_exception(error);
            }
        }

        AWL_ASSERT(finished == count && active == 0);
    });
}

AWL_TEST(AsyncMutex_ReleasesOnException)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::AsyncMutex mutex(executor);
        bool caught = false;

        try
        {
            const auto lock = co_await mutex.asyncLock();
            co_await asio::post(executor, asio::use_awaitable);
            throw std::runtime_error("critical section failed");
        }
        catch (const std::runtime_error&)
        {
            caught = true;
        }

        const auto lock = co_await mutex.asyncLock();
        AWL_ASSERT(caught && lock.ownsLock());
    });
}

AWL_TEST(AsyncMutex_MoveTransfersOwnership)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::AsyncMutex first(executor);
        awl::AsyncMutex second(executor);
        auto original = co_await first.asyncLock();
        awl::AsyncMutex::Lock moved(std::move(original));
        AWL_ASSERT(!original.ownsLock() && moved.ownsLock());
        original.unlock();
        auto destination = co_await second.asyncLock();
        destination = std::move(moved);
        AWL_ASSERT(!moved.ownsLock() && destination.ownsLock());
        const auto second_lock = co_await second.asyncLock();
        destination.unlock();
        destination.unlock();
        const auto first_lock = co_await first.asyncLock();
        AWL_ASSERT(first_lock.ownsLock() && second_lock.ownsLock());
    });
}

AWL_TEST(AsyncMutex_CancelledWaiterDoesNotUnlockOwner)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::AsyncMutex mutex(executor);
        auto owner = co_await mutex.asyncLock();
        asio::cancellation_signal stop;
        awl::Channel<void(boost::system::error_code, std::exception_ptr)> cancelled(executor, 1);
        asio::co_spawn(executor, mutex.asyncLock(), asio::bind_cancellation_slot(stop.slot(),
            [&cancelled](const std::exception_ptr error, awl::AsyncMutex::Lock)
            {
                cancelled.try_send(boost::system::error_code{}, error);
            }));
        co_await asio::post(executor, asio::use_awaitable);
        stop.emit(asio::cancellation_type::terminal);
        AWL_ASSERT(isCancellation(co_await cancelled.async_receive(asio::use_awaitable)));
        bool acquired = false;
        awl::Channel<void(boost::system::error_code, std::exception_ptr)> completed(executor, 1);
        asio::co_spawn(executor, [&]() -> asio::awaitable<void>
        {
            const auto lock = co_await mutex.asyncLock();
            acquired = true;
        }, [&completed](const std::exception_ptr error)
        {
            completed.try_send(boost::system::error_code{}, error);
        });
        co_await asio::post(executor, asio::use_awaitable);
        AWL_ASSERT(!acquired && owner.ownsLock());
        owner.unlock();
        const auto error = co_await completed.async_receive(asio::use_awaitable);

        if (error)
        {
            std::rethrow_exception(error);
        }

        AWL_ASSERT(acquired);
    });
}

AWL_TEST(AsyncMutex_CancelledOwnerReleasesLock)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::AsyncMutex mutex(executor);
        asio::cancellation_signal stop;
        awl::Channel<void(boost::system::error_code)> entered(executor, 1);
        awl::Channel<void(boost::system::error_code, std::exception_ptr)> completed(executor, 1);
        asio::co_spawn(executor, [&]() -> asio::awaitable<void>
        {
            const auto lock = co_await mutex.asyncLock();
            asio::steady_timer timer(executor, asio::steady_timer::time_point::max());
            entered.try_send(boost::system::error_code{});
            co_await timer.async_wait(asio::use_awaitable);
        }, asio::bind_cancellation_slot(stop.slot(), [&completed](const std::exception_ptr error)
        {
            completed.try_send(boost::system::error_code{}, error);
        }));
        co_await entered.async_receive(asio::use_awaitable);
        stop.emit(asio::cancellation_type::terminal);
        AWL_ASSERT(isCancellation(co_await completed.async_receive(asio::use_awaitable)));
        const auto lock = co_await mutex.asyncLock();
        AWL_ASSERT(lock.ownsLock());
    });
}

AWL_TEST(AsyncMutex_CancellationDuringHandoffPreservesToken)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::AsyncMutex mutex(executor);

        for (int i = 0; i < 16; ++i)
        {
            auto owner = co_await mutex.asyncLock();
            asio::cancellation_signal stop;
            awl::Channel<void(boost::system::error_code, std::exception_ptr)> completed(executor, 1);
            asio::co_spawn(executor, mutex.asyncLock(), asio::bind_cancellation_slot(stop.slot(),
                [&completed](const std::exception_ptr error, awl::AsyncMutex::Lock)
                {
                    completed.try_send(boost::system::error_code{}, error);
                }));
            co_await asio::post(executor, asio::use_awaitable);
            owner.unlock();
            stop.emit(asio::cancellation_type::terminal);
            const auto error = co_await completed.async_receive(asio::use_awaitable);
            AWL_ASSERT(!error || isCancellation(error));
            const auto lock = co_await mutex.asyncLock();
            AWL_ASSERT(lock.ownsLock());
        }
    });
}

AWL_TEST(AsyncMutex_LazyWaitAndLockOwnState)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        std::unique_ptr<awl::AsyncMutex> mutex = std::make_unique<awl::AsyncMutex>(executor);
        auto owner = co_await mutex->asyncLock();
        auto waiting = mutex->asyncLock();
        mutex.reset();
        owner.unlock();
        const auto lock = co_await std::move(waiting);
        AWL_ASSERT(lock.ownsLock());
    });
}

AWL_TEST(AsyncMutex_SerializesDifferentWorkerThreads)
{
    AWL_UNUSED_CONTEXT;
    asio::io_context io;
    awl::AsyncMutex mutex(io.get_executor());
    std::atomic<int> active = 0;
    std::atomic<int> completed = 0;
    int updates = 0;
    std::mutex failure_mutex;
    std::exception_ptr failure;
    constexpr int count = 8;

    for (int i = 0; i < count; ++i)
    {
        const auto executor = asio::make_strand(io);
        asio::co_spawn(executor, [&, executor]() -> asio::awaitable<void>
        {
            for (int j = 0; j < 32; ++j)
            {
                const auto lock = co_await mutex.asyncLock();
                AWL_ASSERT(active.fetch_add(1) == 0);
                const int previous = updates;
                co_await asio::post(executor, asio::use_awaitable);
                AWL_ASSERT(active.load() == 1);
                updates = previous + 1;
                AWL_ASSERT(active.fetch_sub(1) == 1);
            }
        }, [&](const std::exception_ptr error)
        {
            if (error)
            {
                const std::lock_guard lock(failure_mutex);
                failure = error;
            }

            ++completed;
        });
    }

    {
        std::jthread first([&io] { io.run(); });
        std::jthread second([&io] { io.run(); });
    }

    if (failure)
    {
        std::rethrow_exception(failure);
    }

    AWL_ASSERT(completed == count && updates == count * 32 && active == 0);
}
