#include "BoostExtras/SignalAwaitable.h"

#include "Awl/Source.h"
#include "Awl/Testing/UnitTest.h"

#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>

#include <exception>
#include <functional>
#include <memory>
#include <string>

namespace
{
    namespace asio = boost::asio;

    void runScenario(std::function<asio::awaitable<void>(asio::any_io_executor)> scenario)
    {
        asio::io_context io;
        const auto executor = asio::make_strand(io);
        std::exception_ptr failure;
        asio::co_spawn(executor, scenario(executor), [&](const std::exception_ptr error)
        {
            failure = error;
        });
        io.run();

        if (failure)
        {
            std::rethrow_exception(failure);
        }
    }
}

AWL_TEST(SignalAwaitable_SingleArgument)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<int> source;
        asio::post(executor, [&source]
        {
            AWL_ASSERT(source.size() == 1);
            source.emit(7);
            AWL_ASSERT(source.empty());
        });
        AWL_ASSERT((co_await awl::asyncWaitSignal(executor, source.signal())) == 7);
        AWL_ASSERT(source.empty());
    });
}

AWL_TEST(SignalAwaitable_Tuple)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<const std::shared_ptr<int>&, const std::string&> source;
        std::shared_ptr<int> expected = std::make_shared<int>(42);
        asio::post(executor, [&source, expected]
        {
            std::string text = "original";
            source.emit(expected, text);
            text = "changed";
        });
        const auto [sender, value] = co_await awl::asyncWaitSignal(executor, source.signal());
        AWL_ASSERT(sender == expected && value == "original");
        AWL_ASSERT(source.empty());
    });
}

AWL_TEST(SignalAwaitable_Void)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<> source;
        asio::post(executor, [&source] { source.emit(); });
        co_await awl::asyncWaitSignal(executor, source.signal());
        AWL_ASSERT(source.empty());
    });
}

AWL_TEST(SignalAwaitable_ResumesOnce)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<int> source;
        bool callback_finished = false;
        asio::post(executor, [&]
        {
            source.emit(11);
            source.emit(12);
            AWL_ASSERT(source.empty());
            callback_finished = true;
        });
        const int value = co_await awl::asyncWaitSignal(executor, source.signal());
        AWL_ASSERT(value == 11 && callback_finished);
    });
}

AWL_TEST(SignalAwaitable_LazySubscription)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<int> source;

        {
            auto unused = awl::asyncWaitSignal(executor, source.signal());
            AWL_ASSERT(source.empty());
        }

        auto operation = awl::asyncWaitSignal(executor, source.signal());
        source.emit(21);
        asio::post(executor, [&source] { source.emit(22); });
        AWL_ASSERT((co_await std::move(operation)) == 22);
        AWL_ASSERT(source.empty());
    });
}

AWL_TEST(SignalAwaitable_UnsubscribesOnCancel)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<int> source;
        asio::cancellation_signal cancellation;
        bool done = false;
        std::exception_ptr failure;
        asio::co_spawn(executor, awl::asyncWaitSignal(executor, source.signal()),
            asio::bind_cancellation_slot(cancellation.slot(), [&](const std::exception_ptr error, int)
            {
                failure = error;
                done = true;
            }));
        co_await asio::post(executor, asio::use_awaitable);
        AWL_ASSERT(source.size() == 1);
        cancellation.emit(asio::cancellation_type::terminal);

        while (!done)
        {
            co_await asio::post(executor, asio::use_awaitable);
        }

        AWL_ASSERT(source.empty() && failure != nullptr);

        try
        {
            std::rethrow_exception(failure);
        }
        catch (const boost::system::system_error& error)
        {
            AWL_ASSERT(error.code() == asio::error::operation_aborted);
        }

        source.emit(99);
        asio::post(executor, [&source] { source.emit(31); });
        AWL_ASSERT((co_await awl::asyncWaitSignal(executor, source.signal())) == 31);
    });
}

AWL_TEST(SignalAwaitable_IndependentWaiters)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<int> source;
        asio::cancellation_signal cancellation;
        bool first_done = false;
        std::exception_ptr first_failure;
        asio::co_spawn(executor, awl::asyncWaitSignal(executor, source.signal()),
            asio::bind_cancellation_slot(cancellation.slot(), [&](const std::exception_ptr error, int)
            {
                first_failure = error;
                first_done = true;
            }));
        co_await asio::post(executor, asio::use_awaitable);
        asio::post(executor, [&]
        {
            AWL_ASSERT(source.size() == 2);
            cancellation.emit(asio::cancellation_type::terminal);
            // The cancelled wait can still be subscribed until its coroutine resumes.
            source.emit(41);
        });
        AWL_ASSERT((co_await awl::asyncWaitSignal(executor, source.signal())) == 41);

        while (!first_done)
        {
            co_await asio::post(executor, asio::use_awaitable);
        }

        AWL_ASSERT(first_failure != nullptr && source.empty());
    });
}
