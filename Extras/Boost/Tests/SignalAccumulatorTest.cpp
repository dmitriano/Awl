#include "BoostExtras/SignalAccumulator.h"
#include "BoostExtras/SignalRangeAccumulator.h"

#include "Awl/ObservableSet.h"
#include "Awl/ObservableUnorderedSet.h"
#include "Awl/Source.h"
#include "Awl/Testing/UnitTest.h"

#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/post.hpp>

#include <exception>
#include <functional>
#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

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

    struct Sender
    {
        explicit Sender(const int id) : id(id)
        {}

        awl::ISignal<int>& changedSignal() { return changed.signal(); }

        awl::ISignal<>& closedSignal() { return closed.signal(); }

        int id;
        awl::Source<int> changed;
        awl::Source<> closed;
    };

    template <class Set>
    asio::awaitable<void> asyncObservable(const asio::any_io_executor executor)
    {
        Set objects;
        std::shared_ptr<Sender> first = std::make_shared<Sender>(1);
        std::shared_ptr<Sender> second = std::make_shared<Sender>(2);
        objects.insert(first);

        {
            awl::SignalRangeAccumulator<Sender, &Sender::changedSignal> accumulator(executor, objects);
            objects.insert(second);
            AWL_ASSERT(first->changed.size() == 1 && second->changed.size() == 1);
            first->changed.emit(11);
            objects.erase(first);
            AWL_ASSERT(first->changed.empty());
            first->changed.emit(99);
            second->changed.emit(12);
            auto [sender1, value1] = co_await accumulator.asyncWait();
            auto [sender2, value2] = co_await accumulator.asyncWait();
            AWL_ASSERT(sender1 == first && value1 == 11);
            AWL_ASSERT(sender2 == second && value2 == 12);
            objects.clear();
            AWL_ASSERT(second->changed.empty());
            objects.insert(first);
            first->changed.emit(13);
            auto [sender3, value3] = co_await accumulator.asyncWait();
            AWL_ASSERT(sender3 == first && value3 == 13);
        }

        AWL_ASSERT(first->changed.empty());
        objects.insert(second);
        AWL_ASSERT(second->changed.empty());
    }
}

AWL_TEST(SignalAccumulator_QueuesInOrder)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<int> source;

        {
            auto accumulator = awl::accumulate_signal(executor, source.signal());
            source.emit(11);
            source.emit(12);
            AWL_ASSERT((co_await accumulator.asyncWait()) == 11);
            // Emissions during processing remain queued behind earlier events.
            source.emit(13);
            AWL_ASSERT((co_await accumulator.asyncWait()) == 12);
            AWL_ASSERT((co_await accumulator.asyncWait()) == 13);
            AWL_ASSERT(source.size() == 1);
        }

        AWL_ASSERT(source.empty());
    });
}

AWL_TEST(SignalAccumulator_AsyncWait)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<int> source;
        awl::SignalAccumulator<int> accumulator(executor, source.signal());
        asio::post(executor, [&source] { source.emit(21); });
        AWL_ASSERT((co_await accumulator.asyncWait()) == 21);
        asio::post(executor, [&source] { source.emit(22); });
        AWL_ASSERT((co_await accumulator.asyncWait()) == 22);
    });
}

AWL_TEST(SignalAccumulator_VoidAndTuple)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<> source;
        awl::SignalAccumulator<> accumulator(executor, source.signal());
        source.emit();
        source.emit();
        co_await accumulator.asyncWait();
        co_await accumulator.asyncWait();
        asio::post(executor, [&source] { source.emit(); });
        co_await accumulator.asyncWait();
        awl::Source<const std::shared_ptr<int>&, const std::string&> tuples;
        auto tuple_accumulator = awl::accumulate_signal(executor, tuples.signal());
        std::shared_ptr<int> object = std::make_shared<int>(42);
        std::string text = "original";
        tuples.emit(object, text);
        text = "changed";
        const auto [received, value] = co_await tuple_accumulator.asyncWait();
        AWL_ASSERT(received == object && value == "original");
    });
}

AWL_TEST(SignalAccumulator_CancellationPreservesEvents)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<int> source;
        awl::SignalAccumulator<int> accumulator(executor, source.signal());
        asio::cancellation_signal cancellation;
        bool done = false;
        std::exception_ptr failure;
        asio::co_spawn(executor, accumulator.asyncWait(), asio::bind_cancellation_slot(cancellation.slot(),
            [&](const std::exception_ptr error, int)
            {
                failure = error;
                done = true;
            }));
        co_await asio::post(executor, asio::use_awaitable);
        source.emit(31);
        cancellation.emit(asio::cancellation_type::terminal);

        while (!done)
        {
            co_await asio::post(executor, asio::use_awaitable);
        }

        AWL_ASSERT(failure != nullptr);

        try
        {
            std::rethrow_exception(failure);
        }
        catch (const boost::system::system_error& error)
        {
            AWL_ASSERT(error.code() == asio::error::operation_aborted);
        }

        AWL_ASSERT(source.size() == 1);
        AWL_ASSERT((co_await accumulator.asyncWait()) == 31);
    });
}

AWL_TEST(SignalAccumulator_DestructionCancelsWait)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<int> source;
        auto accumulator = std::make_unique<awl::SignalAccumulator<int>>(executor, source.signal());
        bool done = false;
        std::exception_ptr failure;
        asio::co_spawn(executor, accumulator->asyncWait(), [&](const std::exception_ptr error, int)
        {
            failure = error;
            done = true;
        });
        co_await asio::post(executor, asio::use_awaitable);
        accumulator.reset();
        AWL_ASSERT(source.empty());
        source.emit(99);

        while (!done)
        {
            co_await asio::post(executor, asio::use_awaitable);
        }

        AWL_ASSERT(failure != nullptr);

        try
        {
            std::rethrow_exception(failure);
        }
        catch (const boost::system::system_error& error)
        {
            AWL_ASSERT(error.code() == asio::error::operation_aborted);
        }
    });
}

AWL_TEST(SignalAccumulator_RejectsConcurrentWaiters)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<int> source;
        awl::SignalAccumulator<int> accumulator(executor, source.signal());
        bool done = false;
        std::exception_ptr failure;
        asio::co_spawn(executor, accumulator.asyncWait(), [&](const std::exception_ptr error, const int value)
        {
            failure = error;
            done = true;
            AWL_ASSERT(value == 41);
        });
        co_await asio::post(executor, asio::use_awaitable);
        bool rejected = false;

        try
        {
            co_await accumulator.asyncWait();
        }
        catch (const std::logic_error&)
        {
            rejected = true;
        }

        AWL_ASSERT(rejected);
        source.emit(41);

        while (!done)
        {
            co_await asio::post(executor, asio::use_awaitable);
        }

        AWL_ASSERT(!failure);
    });
}

AWL_TEST(SignalRangeAccumulator_ObservableSet)
{
    AWL_UNUSED_CONTEXT;
    runScenario(asyncObservable<awl::observable_vector_set<std::shared_ptr<Sender>>>);
}

AWL_TEST(SignalRangeAccumulator_ObservableUnorderedSet)
{
    AWL_UNUSED_CONTEXT;
    runScenario(asyncObservable<awl::observable_unordered_set<std::shared_ptr<Sender>>>);
}

AWL_TEST(SignalRangeAccumulator_FilterViewAndVoid)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        std::shared_ptr<Sender> first = std::make_shared<Sender>(1);
        std::shared_ptr<Sender> second = std::make_shared<Sender>(2);
        std::vector objects{first, second};

        {
            auto accumulator = awl::accumulate_signal<&Sender::changedSignal>(executor,
                objects | std::views::filter([](const auto& sender) { return sender->id == 2; }));
            AWL_ASSERT(first->changed.empty() && second->changed.size() == 1);
            first->changed.emit(99);
            second->changed.emit(51);
            second->changed.emit(52);
            auto [sender1, value1] = co_await accumulator.asyncWait();
            auto [sender2, value2] = co_await accumulator.asyncWait();
            AWL_ASSERT(sender1 == second && sender2 == second && value1 == 51 && value2 == 52);
        }

        AWL_ASSERT(second->changed.empty());

        {
            auto accumulator = awl::accumulate_signal<&Sender::closedSignal>(executor, objects);
            asio::post(executor, [first] { first->closed.emit(); });
            AWL_ASSERT((co_await accumulator.asyncWait()) == first);
        }

        AWL_ASSERT(first->closed.empty() && second->closed.empty());
    });
}

AWL_TEST(SignalRangeAccumulator_CancelAndDestroy)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::observable_vector_set<std::shared_ptr<Sender>> objects;
        std::shared_ptr<Sender> sender = std::make_shared<Sender>(1);
        objects.insert(sender);
        using Accumulator = awl::SignalRangeAccumulator<Sender, &Sender::changedSignal>;
        auto accumulator = std::make_unique<Accumulator>(executor, objects);
        asio::cancellation_signal cancellation;
        bool done = false;
        std::exception_ptr failure;
        asio::co_spawn(executor, accumulator->asyncWait(), asio::bind_cancellation_slot(cancellation.slot(),
            [&](const std::exception_ptr error, std::tuple<std::shared_ptr<Sender>, int>)
            {
                failure = error;
                done = true;
            }));
        co_await asio::post(executor, asio::use_awaitable);
        cancellation.emit(asio::cancellation_type::terminal);

        while (!done)
        {
            co_await asio::post(executor, asio::use_awaitable);
        }

        AWL_ASSERT(failure != nullptr && sender->changed.size() == 1);
        sender->changed.emit(61);
        auto [received, value] = co_await accumulator->asyncWait();
        AWL_ASSERT(received == sender && value == 61);
        done = false;
        failure = nullptr;
        asio::co_spawn(executor, accumulator->asyncWait(),
            [&](const std::exception_ptr error, std::tuple<std::shared_ptr<Sender>, int>)
            {
                failure = error;
                done = true;
            });
        co_await asio::post(executor, asio::use_awaitable);
        accumulator.reset();
        AWL_ASSERT(sender->changed.empty());
        objects.clear();
        objects.insert(sender);
        AWL_ASSERT(sender->changed.empty());

        while (!done)
        {
            co_await asio::post(executor, asio::use_awaitable);
        }

        AWL_ASSERT(failure != nullptr);
    });
}

AWL_TEST(SignalAccumulator_LazyWaitAfterDestruction)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<int> source;
        auto accumulator = std::make_unique<awl::SignalAccumulator<int>>(executor, source.signal());
        source.emit(71);
        auto operation = accumulator->asyncWait();
        accumulator.reset();
        AWL_ASSERT(source.empty());
        bool cancelled = false;

        try
        {
            co_await std::move(operation);
        }
        catch (const boost::system::system_error& error)
        {
            cancelled = error.code() == asio::error::operation_aborted;
        }

        AWL_ASSERT(cancelled);
    });
}

AWL_TEST(SignalRangeAccumulator_RetainsQueuedSender)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::observable_vector_set<std::shared_ptr<Sender>> objects;
        std::shared_ptr<Sender> sender = std::make_shared<Sender>(1);
        std::weak_ptr<Sender> weak = sender;
        objects.insert(sender);
        awl::SignalRangeAccumulator<Sender, &Sender::changedSignal> accumulator(executor, objects);
        sender->changed.emit(81);
        objects.clear();
        sender.reset();
        AWL_ASSERT(!weak.expired());

        {
            auto [received, value] = co_await accumulator.asyncWait();
            AWL_ASSERT(received->id == 1 && value == 81);
            AWL_ASSERT(received->changed.empty());
        }

        AWL_ASSERT(weak.expired());
    });
}

AWL_TEST(SignalAccumulator_EmptyWaitCancellation)
{
    AWL_UNUSED_CONTEXT;
    runScenario([](const asio::any_io_executor executor) -> asio::awaitable<void>
    {
        awl::Source<> source;
        awl::SignalAccumulator<> accumulator(executor, source.signal());
        asio::cancellation_signal cancellation;
        bool done = false;
        std::exception_ptr failure;
        asio::co_spawn(executor, accumulator.asyncWait(), asio::bind_cancellation_slot(cancellation.slot(),
            [&](const std::exception_ptr error)
            {
                failure = error;
                done = true;
            }));
        co_await asio::post(executor, asio::use_awaitable);
        cancellation.emit(asio::cancellation_type::terminal);

        while (!done)
        {
            co_await asio::post(executor, asio::use_awaitable);
        }

        AWL_ASSERT(failure != nullptr);

        try
        {
            std::rethrow_exception(failure);
        }
        catch (const boost::system::system_error& error)
        {
            AWL_ASSERT(error.code() == asio::error::operation_aborted);
        }

        source.emit();
        co_await accumulator.asyncWait();
    });
}
