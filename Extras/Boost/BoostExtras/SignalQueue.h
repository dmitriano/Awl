#pragma once

#include "Awl/ScopeGuard.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/system_error.hpp>

#include <deque>
#include <memory>
#include <stdexcept>
#include <utility>

namespace awl::detail
{
    // Signal callbacks cannot await backpressure. Preserve every event in an unbounded FIFO.
    // Construction, push, destruction and signal delivery require the supplied strand.
    template <class Result>
    class SignalQueue
    {
    private:

        struct State
        {
            explicit State(const boost::asio::any_io_executor executor) :
                executor(executor), wake(executor)
            {}

            const boost::asio::any_io_executor executor;
            boost::asio::steady_timer wake;
            std::deque<Result> results;
            bool waiting = false;
            bool closed = false;
        };

    public:

        explicit SignalQueue(const boost::asio::any_io_executor executor) :
            _state(std::make_shared<State>(executor))
        {}

        SignalQueue(const SignalQueue&) = delete;

        SignalQueue& operator = (const SignalQueue&) = delete;

        ~SignalQueue()
        {
            _state->closed = true;
            _state->results.clear();
            _state->wake.cancel();
        }

        template <class... Args>
        void push(Args&&... args)
        {
            _state->results.emplace_back(std::forward<Args>(args)...);
            _state->wake.cancel_one();
        }

        // Capture state before returning a lazy awaitable; destruction of the owner is safe.
        boost::asio::awaitable<Result> asyncWait()
        {
            return asyncWaitImpl(_state);
        }

    private:

        static boost::asio::awaitable<Result> asyncWaitImpl(std::shared_ptr<State> state)
        {
            namespace asio = boost::asio;
            co_await asio::dispatch(state->executor, asio::use_awaitable);

            if (state->waiting)
            {
                throw std::logic_error("A signal accumulator supports one active waiter.");
            }

            state->waiting = true;
            const auto guard = awl::make_scope_guard([&state] { state->waiting = false; });

            for (;;)
            {
                const auto cancellation = co_await asio::this_coro::cancellation_state;

                if (state->closed || cancellation.cancelled() != asio::cancellation_type::none)
                {
                    throw boost::system::system_error(asio::error::operation_aborted);
                }

                if (!state->results.empty())
                {
                    Result result = std::move(state->results.front());
                    state->results.pop_front();
                    co_return result;
                }

                state->wake.expires_at(asio::steady_timer::time_point::max());
                boost::system::error_code error;
                co_await state->wake.async_wait(asio::redirect_error(asio::use_awaitable, error));

                if (error && error != asio::error::operation_aborted)
                {
                    throw boost::system::system_error(error);
                }
            }
        }

        std::shared_ptr<State> _state;
    };
}
