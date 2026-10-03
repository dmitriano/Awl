#include "BoostExtras/StopToken.h"

#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>

#include <chrono>
#include <exception>
#include <functional>
#include <memory>
#include <stop_token>
#include <thread>
#include <vector>

namespace
{
    namespace asio = boost::asio;

    using Strand = asio::strand<asio::io_context::executor_type>;
    using Channel = asio::experimental::concurrent_channel<void(boost::system::error_code, int)>;

    struct Outcome
    {
        boost::system::error_code error;
        std::exception_ptr exception;
        std::weak_ptr<awl::StopToken> cancellation;
        std::size_t starts = 0;
        std::size_t completions = 0;
        bool onStrand = false;
    };

    asio::awaitable<void> asyncRunIfNotStopped(const std::stop_token stop_token,
        Outcome& outcome, const Strand& executor, asio::awaitable<void> task)
    {
        // A cancellation_signal does not remember an earlier request.
        if (stop_token.stop_requested())
        {
            outcome.error = asio::error::operation_aborted;
            co_return;
        }

        ++outcome.starts;
        outcome.onStrand = executor.running_in_this_thread();
        co_await std::move(task);
    }

    class CancellationScenario
    {
    public:

        explicit CancellationScenario(const std::size_t task_count = 1) :
            _executor(asio::make_strand(_context)),
            _watchdog(_executor, std::chrono::seconds(5)),
            _outcomes(task_count)
        {
            // Only a failed test stops io_context; normal shutdown waits for
            // every co_spawn completion and cancels this watchdog.
            _watchdog.async_wait([this](const boost::system::error_code error)
            {
                if (!error)
                {
                    _timedOut = true;
                    _context.stop();
                }
            });
        }

        const Strand& executor() const
        {
            return _executor;
        }

        Outcome& outcome(const std::size_t index = 0)
        {
            return _outcomes.at(index);
        }

        void spawn(const std::stop_token stop_token, asio::awaitable<void> task,
            const std::size_t index = 0)
        {
            asio::post(_executor, [this, stop_token, index, task = std::move(task)]() mutable
            {
                std::shared_ptr<awl::StopToken> cancellation =
                    std::make_shared<awl::StopToken>(_executor, stop_token);
                Outcome& outcome = _outcomes.at(index);
                outcome.cancellation = cancellation;

                asio::co_spawn(_executor,
                    asyncRunIfNotStopped(stop_token, outcome, _executor, std::move(task)),
                    asio::bind_cancellation_slot(cancellation->slot(),
                        [this, index, cancellation](const std::exception_ptr exception)
                        {
                            Outcome& outcome = _outcomes.at(index);
                            outcome.exception = exception;
                            ++outcome.completions;
                            if (++_completedCount == _outcomes.size())
                            {
                                _watchdog.cancel();
                            }
                        }));
            });
        }

        void run()
        {
            _context.run();
            AWL_ASSERT_FALSE(_timedOut);
            AWL_ASSERT_EQUAL(_outcomes.size(), _completedCount);
            for (const Outcome& outcome : _outcomes)
            {
                if (outcome.exception)
                {
                    std::rethrow_exception(outcome.exception);
                }

                AWL_ASSERT_EQUAL(std::size_t{1}, outcome.completions);
                AWL_ASSERT(outcome.cancellation.expired());
            }
        }

    private:

        asio::io_context _context;
        Strand _executor;
        asio::steady_timer _watchdog;
        std::vector<Outcome> _outcomes;
        std::size_t _completedCount = 0;
        bool _timedOut = false;
    };

    asio::awaitable<void> asyncWaitTimer(Outcome& outcome, const std::function<void()> before_wait)
    {
        asio::steady_timer timer(co_await asio::this_coro::executor);
        timer.expires_at(asio::steady_timer::time_point::max());
        before_wait();
        co_await timer.async_wait(asio::redirect_error(asio::use_awaitable, outcome.error));
    }

    asio::awaitable<void> asyncReceive(Channel& channel, Outcome& outcome,
        const std::function<void()> before_wait, int& value)
    {
        before_wait();
        value = co_await channel.async_receive(asio::redirect_error(asio::use_awaitable, outcome.error));
    }

    asio::awaitable<void> asyncSend(Channel& channel, Outcome& outcome,
        const std::function<void()> before_wait)
    {
        before_wait();
        co_await channel.async_send(boost::system::error_code{}, 2,
            asio::redirect_error(asio::use_awaitable, outcome.error));
    }

    asio::awaitable<void> asyncFinish()
    {
        co_await asio::post(asio::use_awaitable);
    }

    void assertStarted(const Outcome& outcome)
    {
        AWL_ASSERT_EQUAL(std::size_t{1}, outcome.starts);
        AWL_ASSERT(outcome.onStrand);
    }
}

AWL_TEST(AsioCancellationTimer)
{
    AWL_UNUSED_CONTEXT;

    CancellationScenario scenario;
    std::stop_source source;
    bool first_request = false;
    bool repeated_request = true;
    scenario.spawn(source.get_token(), asyncWaitTimer(scenario.outcome(), [&]
    {
        // This handler cannot run until async_wait has installed its handler.
        asio::post(scenario.executor(), [&]
        {
            first_request = source.request_stop();
            repeated_request = source.request_stop();
        });
    }));
    scenario.run();

    assertStarted(scenario.outcome());
    AWL_ASSERT(scenario.outcome().error == asio::error::operation_aborted);
    AWL_ASSERT(first_request);
    AWL_ASSERT_FALSE(repeated_request);
    AWL_ASSERT_FALSE(source.request_stop());
}

AWL_TEST(AsioCancellationBeforeLaunch)
{
    AWL_UNUSED_CONTEXT;

    CancellationScenario scenario;
    std::stop_source source;
    source.request_stop();
    scenario.spawn(source.get_token(), asyncFinish());
    scenario.run();

    AWL_ASSERT_EQUAL(std::size_t{0}, scenario.outcome().starts);
    AWL_ASSERT(scenario.outcome().error == asio::error::operation_aborted);
}

AWL_TEST(AsioCancellationBeforeRun)
{
    AWL_UNUSED_CONTEXT;

    CancellationScenario scenario;
    std::stop_source source;
    scenario.spawn(source.get_token(), asyncFinish());
    source.request_stop();
    scenario.run();

    AWL_ASSERT_EQUAL(std::size_t{0}, scenario.outcome().starts);
    AWL_ASSERT(scenario.outcome().error == asio::error::operation_aborted);
}

AWL_TEST(AsioCancellationBetweenCheckAndWait)
{
    AWL_UNUSED_CONTEXT;

    CancellationScenario scenario;
    std::stop_source source;
    scenario.spawn(source.get_token(), asyncWaitTimer(scenario.outcome(), [&]
    {
        // The guard has passed, but async_wait has not started yet. An inline
        // dispatch here loses the cancellation; posting preserves it.
        AWL_ASSERT(scenario.executor().running_in_this_thread());
        source.request_stop();
    }));
    scenario.run();

    assertStarted(scenario.outcome());
    AWL_ASSERT(scenario.outcome().error == asio::error::operation_aborted);
}

AWL_TEST(AsioCancellationMultipleTasks)
{
    AWL_UNUSED_CONTEXT;

    constexpr std::size_t task_count = 4;
    CancellationScenario scenario(task_count);
    std::stop_source source;
    std::size_t ready_count = 0;
    for (std::size_t index = 0; index != task_count; ++index)
    {
        scenario.spawn(source.get_token(), asyncWaitTimer(scenario.outcome(index), [&]
        {
            if (++ready_count == task_count)
            {
                asio::post(scenario.executor(), [&] { source.request_stop(); });
            }
        }), index);
    }
    scenario.run();

    AWL_ASSERT_EQUAL(task_count, ready_count);
    for (std::size_t index = 0; index != task_count; ++index)
    {
        assertStarted(scenario.outcome(index));
        AWL_ASSERT(scenario.outcome(index).error == asio::error::operation_aborted);
    }
}

AWL_TEST(AsioCancellationChannelReceive)
{
    AWL_UNUSED_CONTEXT;

    CancellationScenario scenario;
    Channel channel(scenario.executor(), 1);
    std::stop_source source;
    int received_value = -1;
    scenario.spawn(source.get_token(), asyncReceive(channel, scenario.outcome(), [&]
    {
        asio::post(scenario.executor(), [&] { source.request_stop(); });
    }, received_value));
    scenario.run();

    assertStarted(scenario.outcome());
    AWL_ASSERT(scenario.outcome().error == asio::experimental::error::channel_cancelled);
    // Cancelling this receive must not close/cancel the entire channel.
    AWL_ASSERT(channel.try_send(boost::system::error_code{}, 7));
    bool delivered = false;
    AWL_ASSERT(channel.try_receive([&](const boost::system::error_code error, const int value)
    {
        AWL_ASSERT(!error);
        AWL_ASSERT_EQUAL(7, value);
        delivered = true;
    }));
    AWL_ASSERT(delivered);
}

AWL_TEST(AsioCancellationChannelSend)
{
    AWL_UNUSED_CONTEXT;

    CancellationScenario scenario;
    Channel channel(scenario.executor(), 1);
    AWL_ASSERT(channel.try_send(boost::system::error_code{}, 1));
    std::stop_source source;
    scenario.spawn(source.get_token(), asyncSend(channel, scenario.outcome(), [&]
    {
        asio::post(scenario.executor(), [&] { source.request_stop(); });
    }));
    scenario.run();

    assertStarted(scenario.outcome());
    AWL_ASSERT(scenario.outcome().error == asio::experimental::error::channel_cancelled);
    AWL_ASSERT(channel.try_receive([](const boost::system::error_code error, const int value)
    {
        AWL_ASSERT(!error);
        AWL_ASSERT_EQUAL(1, value);
    }));
    AWL_ASSERT_FALSE(channel.try_receive([](boost::system::error_code, int)
    {
        AWL_FAIL;
    }));
}

AWL_TEST(AsioCancellationFromOtherThread)
{
    AWL_UNUSED_CONTEXT;

    CancellationScenario scenario;
    std::stop_source source;
    bool requested_on_other_thread = false;
    scenario.spawn(source.get_token(), asyncWaitTimer(scenario.outcome(), [&]
    {
        asio::post(scenario.executor(), [&]
        {
            const std::thread::id executor_thread = std::this_thread::get_id();
            std::jthread requester([&]
            {
                requested_on_other_thread = std::this_thread::get_id() != executor_thread;
                source.request_stop();
            });
        });
    }));
    scenario.run();

    AWL_ASSERT(requested_on_other_thread);
    assertStarted(scenario.outcome());
    AWL_ASSERT(scenario.outcome().error == asio::error::operation_aborted);
}

AWL_TEST(AsioCancellationAfterCompletion)
{
    AWL_UNUSED_CONTEXT;

    CancellationScenario scenario;
    std::stop_source source;
    scenario.spawn(source.get_token(), asyncFinish());
    scenario.run();

    assertStarted(scenario.outcome());
    AWL_ASSERT(!scenario.outcome().error);
    AWL_ASSERT(source.request_stop());
    AWL_ASSERT_FALSE(source.request_stop());
    AWL_ASSERT_EQUAL(std::size_t{1}, scenario.outcome().completions);
}

AWL_TEST(AsioCancellationQueuedAfterDestruction)
{
    AWL_UNUSED_CONTEXT;

    asio::io_context io_context;
    const Strand executor = asio::make_strand(io_context);
    std::stop_source source;
    std::size_t emissions = 0;
    std::weak_ptr<awl::StopToken> weak_cancellation;
    asio::post(executor, [&]
    {
        std::shared_ptr<awl::StopToken> cancellation =
            std::make_shared<awl::StopToken>(executor, source.get_token());
        weak_cancellation = cancellation;
        cancellation->slot().assign([&](asio::cancellation_type) { ++emissions; });
        source.request_stop();
        // The queued emit must safely do nothing after this owner is gone.
        cancellation.reset();
    });
    io_context.run();

    AWL_ASSERT(weak_cancellation.expired());
    AWL_ASSERT_EQUAL(std::size_t{0}, emissions);
}
