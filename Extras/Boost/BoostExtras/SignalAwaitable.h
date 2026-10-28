#pragma once

#include "BoostExtras/SignalAccumulator.h"

#include "Awl/ISignal.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <type_traits>
#include <utility>

namespace awl
{
    namespace detail
    {
        template <class... Args>
        class SignalWaiter
        {
        public:

            using Result = signal_accumulator_result_t<Args...>;
            using StoredResult = signal_accumulator_storage_t<Result>;

            SignalWaiter(const boost::asio::any_io_executor executor, ISignal<Args...>& signal) :
                _signal(signal), _queue(executor)
            {
                _subscriptionId = _signal.subscribe([this](Args... args)
                {
                    _queue.push(std::forward<Args>(args)...);
                    unsubscribe();
                });
            }

            SignalWaiter(const SignalWaiter&) = delete;

            SignalWaiter& operator = (const SignalWaiter&) = delete;

            ~SignalWaiter()
            {
                unsubscribe();
            }

            boost::asio::awaitable<StoredResult> asyncWait()
            {
                return _queue.asyncWait();
            }

        private:

            void unsubscribe()
            {
                if (_subscriptionId != 0)
                {
                    _signal.unsubscribe(_subscriptionId);
                    _subscriptionId = 0;
                }
            }

            ISignal<Args...>& _signal;
            Id _subscriptionId = 0;

            SignalQueue<StoredResult> _queue;
        };
    }

    // Lazy one-shot wait: subscribe when awaited and unsubscribe on the first signal,
    // cancellation or completion. Signal must outlive the operation. Signal delivery
    // and subscription lifetime require the supplied caller-owned strand executor.
    template <class... Args>
    boost::asio::awaitable<detail::signal_accumulator_result_t<Args...>> asyncWaitSignal(
        const boost::asio::any_io_executor executor, ISignal<Args...>& signal)
    {
        co_await boost::asio::dispatch(executor, boost::asio::use_awaitable);
        detail::SignalWaiter<Args...> waiter(executor, signal);

        if constexpr (std::is_void_v<typename detail::SignalWaiter<Args...>::Result>)
        {
            co_await waiter.asyncWait();
        }
        else
        {
            co_return co_await waiter.asyncWait();
        }
    }
}
