#include "BoostExtras/ShutdownSignalFormatter.h"
#include "BoostExtras/ShutdownSignals.h"

#include "Awl/Testing/UnitTest.h"

#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/system_error.hpp>

#include <csignal>
#include <exception>
#include <format>
#include <functional>
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

    void checkCancelled(const std::exception_ptr failure)
    {
        AWL_ASSERT(failure != nullptr);

        try
        {
            std::rethrow_exception(failure);
        }
        catch (const boost::system::system_error& error)
        {
            AWL_ASSERT(error.code() == asio::error::operation_aborted);
        }
    }

    asio::awaitable<void> asyncCancellation(const asio::any_io_executor executor, const bool cancel_object)
    {
        awl::ShutdownSignals signals(executor);
        asio::cancellation_signal stop;
        bool done = false;
        std::exception_ptr failure;
        asio::co_spawn(executor, signals.asyncWait(), asio::bind_cancellation_slot(stop.slot(),
            [&](const std::exception_ptr error, awl::ShutdownSignal)
            {
                failure = error;
                done = true;
            }));
        co_await asio::post(executor, asio::use_awaitable);

        if (cancel_object)
        {
            signals.cancel();
        }
        else
        {
            stop.emit(asio::cancellation_type::terminal);
        }

        while (!done)
        {
            co_await asio::post(executor, asio::use_awaitable);
        }

        checkCancelled(failure);
        asio::post(executor, [] { AWL_ASSERT(std::raise(SIGTERM) == 0); });
        AWL_ASSERT((co_await signals.asyncWait()) == awl::ShutdownSignal::Terminate);
    }
}

AWL_TEST(ShutdownSignals_Interrupt)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::ShutdownSignals signals(executor);
        bool emitted = false;
        asio::post(executor, [&emitted]
        {
            AWL_ASSERT(std::raise(SIGINT) == 0);
            emitted = true;
        });
        AWL_ASSERT((co_await signals.asyncWait()) == awl::ShutdownSignal::Interrupt);
        AWL_ASSERT(emitted);
    });
}

AWL_TEST(ShutdownSignals_QueuedBeforeWait)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::ShutdownSignals signals(executor);
        AWL_ASSERT(std::raise(SIGTERM) == 0);
        AWL_ASSERT((co_await signals.asyncWait()) == awl::ShutdownSignal::Terminate);
    });
}

AWL_TEST(ShutdownSignals_CancelAndReuse)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        co_await asyncCancellation(executor, true);
    });
}

AWL_TEST(ShutdownSignals_CoroutineCancellationAndReuse)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        co_await asyncCancellation(executor, false);
    });
}

AWL_TEST(ShutdownSignals_CancelWithoutWait)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::ShutdownSignals signals(executor);
        signals.cancel();
        asio::post(executor, [] { AWL_ASSERT(std::raise(SIGINT) == 0); });
        AWL_ASSERT((co_await signals.asyncWait()) == awl::ShutdownSignal::Interrupt);
    });
}

AWL_TEST(ShutdownSignals_FormatsNames)
{
    AWL_UNUSED_CONTEXT;
    AWL_ASSERT(std::format("{}", awl::ShutdownSignal::Interrupt) == "Ctrl+C (SIGINT)");
    AWL_ASSERT(std::format("{}", awl::ShutdownSignal::Terminate) == "SIGTERM");
    AWL_ASSERT(std::format("{}", awl::ShutdownSignal::Break) == "Ctrl+Break (SIGBREAK)");
    AWL_ASSERT(std::format("{:>9}", awl::ShutdownSignal::Terminate) == "  SIGTERM");
    AWL_ASSERT(std::format(L"{}", awl::ShutdownSignal::Interrupt) == L"Ctrl+C (SIGINT)");
    AWL_ASSERT(std::format(L"{}", awl::ShutdownSignal::Terminate) == L"SIGTERM");
    AWL_ASSERT(std::format(L"{}", awl::ShutdownSignal::Break) == L"Ctrl+Break (SIGBREAK)");
}
