#pragma once

#include "BoostExtras/SignalQueue.h"

#include "Awl/ISignal.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace awl
{
    namespace detail
    {
        template <class... Args>
        struct signal_accumulator_result
        {
            using type = std::tuple<std::decay_t<Args>...>;
        };

        template <class Arg>
        struct signal_accumulator_result<Arg>
        {
            using type = std::decay_t<Arg>;
        };

        template <>
        struct signal_accumulator_result<>
        {
            using type = void;
        };

        template <class... Args>
        using signal_accumulator_result_t = typename signal_accumulator_result<Args...>::type;

        template <class Result>
        struct signal_accumulator_storage
        {
            using type = Result;
        };

        template <>
        struct signal_accumulator_storage<void>
        {
            using type = std::monostate;
        };

        template <class Result>
        using signal_accumulator_storage_t = typename signal_accumulator_storage<Result>::type;
    }

    // All signal delivery, subscription and destruction require executor's strand.
    // The signal must outlive the accumulator. FIFO is unbounded; one active waiter.
    template <class... Args>
    class SignalAccumulator
    {
    private:

        using Result = detail::signal_accumulator_result_t<Args...>;
        using StoredResult = detail::signal_accumulator_storage_t<Result>;

    public:

        SignalAccumulator(const boost::asio::any_io_executor executor, ISignal<Args...>& signal) :
            _signal(signal), _queue(executor)
        {
            _subscriptionId = _signal.subscribe([this](Args... args)
            {
                if constexpr (sizeof...(Args) == 0)
                {
                    _queue.push();
                }
                else
                {
                    _queue.push(std::forward<Args>(args)...);
                }
            });
        }

        SignalAccumulator(const SignalAccumulator&) = delete;

        SignalAccumulator(SignalAccumulator&&) = delete;

        SignalAccumulator& operator = (const SignalAccumulator&) = delete;

        SignalAccumulator& operator = (SignalAccumulator&&) = delete;

        ~SignalAccumulator()
        {
            _signal.unsubscribe(_subscriptionId);
        }

        boost::asio::awaitable<Result> asyncWait()
        {
            if constexpr (std::is_void_v<Result>)
            {
                return asyncWaitVoid(_queue.asyncWait());
            }
            else
            {
                return _queue.asyncWait();
            }
        }

    private:

        static boost::asio::awaitable<void> asyncWaitVoid(boost::asio::awaitable<StoredResult> operation)
        {
            co_await std::move(operation);
        }

        ISignal<Args...>& _signal;
        Id _subscriptionId = 0;
        detail::SignalQueue<StoredResult> _queue;
    };

    template <class... Args>
    SignalAccumulator<Args...> accumulate_signal(const boost::asio::any_io_executor executor, ISignal<Args...>& signal)
    {
        return SignalAccumulator<Args...>(executor, signal);
    }
}
