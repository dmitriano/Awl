#include "BoostExtras/Channel.h"
#include "BoostExtras/ActorFramework/Request.h"
#include "BoostExtras/SpawnTask.h"

#include "Awl/Testing/UnitTest.h"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <variant>
#include <vector>

namespace
{
    namespace asio = boost::asio;

    struct Market
    {
        std::string symbol;
        int price;
    };

    struct MarketRequest
    {
        using Result = Market;

        std::string symbol;
        awl::RequestCompletion<Result> completion;
    };

    struct ValueRequest
    {
        using Result = std::unique_ptr<int>;

        std::unique_ptr<int> value;
        awl::RequestCompletion<Result> completion;
    };

    struct PingRequest
    {
        using Result = void;

        awl::RequestCompletion<Result> completion;
    };

    struct DropRequest
    {
        using Result = int;

        awl::RequestCompletion<Result> completion;
    };

    struct DeferredRequest
    {
        using Result = int;

        awl::RequestCompletion<Result> completion;
    };

    struct FinishRequest
    {
        using Result = void;

        awl::RequestCompletion<Result> completion;
    };

    struct StopMessage {};

    class MarketModel
    {
    public:

        using Message = std::variant<MarketRequest, ValueRequest, PingRequest, DropRequest,
            DeferredRequest, FinishRequest, StopMessage>;
        using Inbox = awl::Channel<void(boost::system::error_code, Message)>;

        explicit MarketModel(const asio::any_io_executor executor) :
            _executor(executor), _inbox(std::make_shared<Inbox>(executor, 2))
        {}

        asio::awaitable<Market> asyncGetMarket(std::string symbol)
        {
            return awl::asyncRequest(_inbox, MarketRequest{std::move(symbol), {}});
        }

        const std::shared_ptr<Inbox>& inbox() const { return _inbox; }

        int marketCalls() const { return _marketCalls.load(); }

        bool hasDeferredRequest() const { return _hasDeferred.load(); }

        asio::awaitable<void> asyncStop()
        {
            co_await _inbox->async_send(boost::system::error_code{}, StopMessage{}, asio::use_awaitable);
        }

        asio::awaitable<void> asyncRun()
        {
            for (;;)
            {
                Message message = co_await _inbox->async_receive(asio::use_awaitable);

                if (std::holds_alternative<StopMessage>(message))
                {
                    break;
                }

                co_await asio::post(asio::use_awaitable);
                AWL_ASSERT((co_await asio::this_coro::executor) == _executor);
                std::visit([this](auto& request) { handle(request); }, message);
            }
        }

    private:

        void handle(MarketRequest& request)
        {
            ++_marketCalls;

            if (request.symbol == "missing")
            {
                AWL_ASSERT(request.completion.fail(
                    std::make_exception_ptr(std::out_of_range("Unknown market."))));
            }
            else
            {
                AWL_ASSERT(request.completion.complete(Market{request.symbol, 123}));
                AWL_ASSERT(!request.completion.complete(Market{request.symbol, 999}));
            }
        }

        void handle(ValueRequest& request)
        {
            AWL_ASSERT(request.completion.complete(std::move(request.value)));
        }

        void handle(PingRequest& request)
        {
            AWL_ASSERT(request.completion.complete({}));
        }

        void handle(DropRequest&)
        {
            // Destroy the unanswered request to exercise the broken-reply path.
        }

        void handle(DeferredRequest& request)
        {
            _deferred = request.completion;
            _hasDeferred = true;
        }

        void handle(FinishRequest& request)
        {
            AWL_ASSERT(_deferred.complete(42));
            _hasDeferred = false;
            AWL_ASSERT(request.completion.complete({}));
        }

        void handle(StopMessage&)
        {}

        const asio::any_io_executor _executor;
        const std::shared_ptr<Inbox> _inbox;
        std::atomic<int> _marketCalls = 0;
        awl::RequestCompletion<int> _deferred;
        std::atomic<bool> _hasDeferred = false;
    };

    // The bot has its own inbox and exactly one consumer. It can await the model
    // from inside its handler: replies do not require processing the bot's inbox.
    class MarketBot
    {
    public:

        using Message = std::variant<MarketRequest, StopMessage>;
        using Inbox = awl::Channel<void(boost::system::error_code, Message)>;

        MarketBot(const asio::any_io_executor executor, MarketModel& model) :
            _executor(executor), _model(model), _inbox(std::make_shared<Inbox>(executor, 2))
        {}

        asio::awaitable<Market> asyncGetMarket(std::string symbol)
        {
            return awl::asyncRequest(_inbox, MarketRequest{std::move(symbol), {}});
        }

        asio::awaitable<void> asyncStop()
        {
            co_await _inbox->async_send(boost::system::error_code{}, StopMessage{}, asio::use_awaitable);
        }

        asio::awaitable<void> asyncRun()
        {
            for (;;)
            {
                Message message = co_await _inbox->async_receive(asio::use_awaitable);

                if (std::holds_alternative<StopMessage>(message))
                {
                    break;
                }

                AWL_ASSERT(_active++ == 0);
                MarketRequest& request = std::get<MarketRequest>(message);

                try
                {
                    Market market = co_await _model.asyncGetMarket(request.symbol);
                    AWL_ASSERT((co_await asio::this_coro::executor) == _executor);
                    AWL_ASSERT(_active == 1);
                    AWL_ASSERT(request.completion.complete(std::move(market)));
                }
                catch (...)
                {
                    request.completion.fail(std::current_exception());
                }

                --_active;
            }
        }

    private:

        const asio::any_io_executor _executor;
        MarketModel& _model;
        const std::shared_ptr<Inbox> _inbox;
        int _active = 0;
    };

    using Scenario = std::function<asio::awaitable<void>(MarketModel&, MarketBot&)>;

    asio::awaitable<void> asyncDeferredRequest(MarketModel& model)
    {
        const int value = co_await awl::asyncRequest(model.inbox(), DeferredRequest{});
        AWL_ASSERT_EQUAL(42, value);
    }

    asio::awaitable<void> asyncScenario(MarketModel& model, MarketBot& bot, Scenario scenario)
    {
        std::exception_ptr failure;

        try
        {
            co_await scenario(model, bot);
        }
        catch (...)
        {
            failure = std::current_exception();
        }

        co_await bot.asyncStop();
        co_await model.asyncStop();

        if (failure)
        {
            std::rethrow_exception(failure);
        }
    }

    void runComponents(const bool separate_strands, Scenario scenario)
    {
        asio::io_context io;
        const auto model_executor = asio::make_strand(io);
        const auto bot_executor = separate_strands ? asio::make_strand(io) : model_executor;
        const auto caller_executor = asio::make_strand(io);
        MarketModel model(model_executor);
        MarketBot bot(bot_executor, model);
        std::array<std::exception_ptr, 3> failures;
        std::array<bool, 3> completed{};
        bool timed_out = false;
        asio::steady_timer deadline(caller_executor, std::chrono::seconds(5));
        deadline.async_wait([&](const boost::system::error_code error)
        {
            if (!error)
            {
                timed_out = true;
                io.stop();
            }
        });

        asio::co_spawn(model_executor, model.asyncRun(), [&](const std::exception_ptr error)
        {
            failures[0] = error;
            completed[0] = true;
        });
        asio::co_spawn(bot_executor, bot.asyncRun(), [&](const std::exception_ptr error)
        {
            failures[1] = error;
            completed[1] = true;
        });
        asio::co_spawn(caller_executor, asyncScenario(model, bot, std::move(scenario)),
            [&](const std::exception_ptr error)
            {
                failures[2] = error;
                completed[2] = true;
                deadline.cancel();
            });

        std::jthread worker([&io] { io.run(); });
        io.run();
        worker.join();
        AWL_ASSERT(!timed_out);

        for (std::size_t i = 0; i != failures.size(); ++i)
        {
            if (failures[i])
            {
                std::rethrow_exception(failures[i]);
            }

            AWL_ASSERT(completed[i]);
        }
    }
}

AWL_TEST(Request_ComponentsOnSameStrand)
{
    AWL_UNUSED_CONTEXT;
    runComponents(false, [](MarketModel& model, MarketBot& bot) -> asio::awaitable<void>
    {
        auto pending = bot.asyncGetMarket("XRPFDUSD");
        AWL_ASSERT(model.marketCalls() == 0);
        Market market = co_await std::move(pending);
        AWL_ASSERT_EQUAL(std::string("XRPFDUSD"), market.symbol);
        AWL_ASSERT_EQUAL(123, market.price);
        market.price = 999;

        const Market next = co_await bot.asyncGetMarket("BTCUSDT");
        AWL_ASSERT_EQUAL(std::string("BTCUSDT"), next.symbol);
        AWL_ASSERT_EQUAL(123, next.price);
        AWL_ASSERT_EQUAL(2, model.marketCalls());
    });
}

AWL_TEST(Request_ConcurrentCallersOnDifferentStrands)
{
    AWL_UNUSED_CONTEXT;
    runComponents(true, [](MarketModel& model, MarketBot& bot) -> asio::awaitable<void>
    {
        const asio::any_io_executor executor = co_await asio::this_coro::executor;
        std::vector<awl::SpawnedTask<Market>> tasks;

        for (int i = 0; i != 20; ++i)
        {
            tasks.push_back(awl::spawnTask(executor, bot.asyncGetMarket(std::to_string(i))));
        }

        for (int i = 0; i != 20; ++i)
        {
            const Market market = co_await std::move(tasks[i]);
            AWL_ASSERT_EQUAL(std::to_string(i), market.symbol);
            AWL_ASSERT_EQUAL(123, market.price);
            AWL_ASSERT((co_await asio::this_coro::executor) == executor);
        }

        AWL_ASSERT_EQUAL(20, model.marketCalls());
    });
}

AWL_TEST(Request_ExceptionAndContinuedProcessing)
{
    AWL_UNUSED_CONTEXT;
    runComponents(true, [](MarketModel&, MarketBot& bot) -> asio::awaitable<void>
    {
        bool caught = false;

        try
        {
            co_await bot.asyncGetMarket("missing");
        }
        catch (const std::out_of_range& error)
        {
            caught = true;
            AWL_ASSERT_EQUAL(std::string("Unknown market."), std::string(error.what()));
        }

        AWL_ASSERT(caught);
        const Market market = co_await bot.asyncGetMarket("XRPFDUSD");
        AWL_ASSERT_EQUAL(123, market.price);
    });
}

AWL_TEST(Request_MoveOnlyPayloadAndVoidReply)
{
    AWL_UNUSED_CONTEXT;
    runComponents(true, [](MarketModel& model, MarketBot&) -> asio::awaitable<void>
    {
        const std::unique_ptr<int> value = co_await awl::asyncRequest(
            model.inbox(), ValueRequest{std::make_unique<int>(42), {}});
        AWL_ASSERT_EQUAL(42, *value);
        co_await awl::asyncRequest(model.inbox(), PingRequest{});
    });
}

AWL_TEST(Request_AbandonedReply)
{
    AWL_UNUSED_CONTEXT;
    runComponents(true, [](MarketModel& model, MarketBot&) -> asio::awaitable<void>
    {
        bool caught = false;

        try
        {
            co_await awl::asyncRequest(model.inbox(), DropRequest{});
        }
        catch (const boost::system::system_error& error)
        {
            caught = error.code() == asio::error::operation_aborted;
        }

        AWL_ASSERT(caught);
    });
}

AWL_TEST(Request_ClosedInbox)
{
    AWL_UNUSED_CONTEXT;
    runComponents(true, [](MarketModel&, MarketBot&) -> asio::awaitable<void>
    {
        const asio::any_io_executor executor = co_await asio::this_coro::executor;
        std::shared_ptr<MarketModel::Inbox> inbox = std::make_shared<MarketModel::Inbox>(executor, 1);
        inbox->close();
        bool caught = false;

        try
        {
            co_await awl::asyncRequest(inbox, MarketRequest{"XRPFDUSD", {}});
        }
        catch (const boost::system::system_error& error)
        {
            caught = error.code() == asio::experimental::error::channel_closed;
        }

        AWL_ASSERT(caught);
    });
}

AWL_TEST(Request_AdmittedRequestFinishesBeforeCancellation)
{
    AWL_UNUSED_CONTEXT;
    runComponents(true, [](MarketModel& model, MarketBot&) -> asio::awaitable<void>
    {
        const asio::any_io_executor executor = co_await asio::this_coro::executor;
        asio::cancellation_signal stop;
        bool completed = false;
        std::exception_ptr failure;
        asio::co_spawn(executor, asyncDeferredRequest(model), asio::bind_cancellation_slot(stop.slot(),
            [&](const std::exception_ptr error)
            {
                failure = error;
                completed = true;
            }));

        while (!model.hasDeferredRequest())
        {
            co_await asio::post(asio::use_awaitable);
        }

        stop.emit(asio::cancellation_type::terminal);
        co_await asio::post(asio::use_awaitable);
        AWL_ASSERT(!completed);
        co_await awl::asyncRequest(model.inbox(), FinishRequest{});

        while (!completed)
        {
            co_await asio::post(asio::use_awaitable);
        }

        if (failure)
        {
            std::rethrow_exception(failure);
        }
    });
}

AWL_TEST(Request_CompletionKeepsExecutorsAlive)
{
    AWL_UNUSED_CONTEXT;
    asio::io_context caller_io;
    asio::io_context target_io;
    const auto caller_executor = asio::make_strand(caller_io);
    const auto target_executor = asio::make_strand(target_io);
    bool completed = false;
    std::exception_ptr failure;
    std::promise<void> caller_started;
    auto started = caller_started.get_future();
    auto completion = awl::RequestCompletion<int>::create(
        asio::bind_executor(caller_executor, [&](awl::RequestCompletion<int>::Response response)
        {
            try
            {
                AWL_ASSERT(caller_executor.running_in_this_thread());
                AWL_ASSERT(!target_executor.running_in_this_thread());
                AWL_ASSERT(response && *response == 42);
                completed = true;
            }
            catch (...)
            {
                failure = std::current_exception();
            }
        }), caller_executor, target_executor);

    asio::post(caller_executor, [&caller_started] { caller_started.set_value(); });
    std::jthread caller([&caller_io] { caller_io.run(); });
    std::jthread target([&target_io] { target_io.run(); });
    const bool ready = started.wait_for(std::chrono::seconds(5)) == std::future_status::ready;

    if (ready)
    {
        asio::post(target_executor, [completion] { completion.complete(42); });
    }
    else
    {
        caller_io.stop();
        target_io.stop();
    }

    caller.join();
    target.join();
    AWL_ASSERT(ready);

    if (failure)
    {
        std::rethrow_exception(failure);
    }

    AWL_ASSERT(completed);
}
