#pragma once

#include "Awl/ISignal.h"

#include <cassert>
#include <coroutine>
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace awl::coro
{
    template <class... Args>
    class SignalAwaitable;

    namespace detail
    {
        template <class... Args>
        struct signal_awaitable_result
        {
            using type = std::tuple<std::decay_t<Args>...>;
        };

        template <class Arg>
        struct signal_awaitable_result<Arg>
        {
            using type = std::decay_t<Arg>;
        };

        template <>
        struct signal_awaitable_result<>
        {
            using type = void;
        };

        template <class... Args>
        using signal_awaitable_result_t = typename signal_awaitable_result<Args...>::type;

        template <class Result>
        struct signal_awaitable_state
        {
            std::optional<Result> result;
        };

        template <>
        struct signal_awaitable_state<void>
        {};

        template <class Result, class State, class... Args>
        void store_signal_awaitable_result(State& state, Args&&... args)
        {
            if constexpr (std::is_void_v<Result>)
            {
                static_cast<void>(sizeof...(args));
            }
            else
            {
                state.result.emplace(std::forward<Args>(args)...);
            }
        }
    }

    template <class... Args>
    class SignalAwaitable
    {
    private:

        using Result = detail::signal_awaitable_result_t<Args...>;

    public:

        explicit SignalAwaitable(ISignal<Args...>& signal) :
            _signal(signal)
        {}

        SignalAwaitable(const SignalAwaitable&) = delete;

        SignalAwaitable(SignalAwaitable&& other) noexcept :
            _signal(other._signal),
            _state(std::move(other._state))
        {
            assert(other._subscriptionId == 0);
            assert(other._coroutine == nullptr);
        }

        SignalAwaitable& operator = (const SignalAwaitable&) = delete;

        SignalAwaitable& operator = (SignalAwaitable&& other) noexcept
        {
            if (this != &other)
            {
                assert(_subscriptionId == 0);
                assert(_coroutine == nullptr);
                assert(other._subscriptionId == 0);
                assert(other._coroutine == nullptr);

                _signal = other._signal;
                _state = std::move(other._state);
            }

            return *this;
        }

        ~SignalAwaitable()
        {
            unsubscribe();
        }

        bool await_ready() const noexcept
        {
            return false;
        }

        void await_suspend(std::coroutine_handle<> h)
        {
            _coroutine = h;

            _subscriptionId = _signal.get().subscribe(
                [this](Args... args)
                {
                    store(std::forward<Args>(args)...);
                    unsubscribe();
                    _coroutine.resume();
                });
        }

        decltype(auto) await_resume()
        {
            if constexpr (std::is_void_v<Result>)
            {
                return;
            }
            else
            {
                return std::move(*_state.result);
            }
        }

    private:

        void unsubscribe()
        {
            if (_subscriptionId != 0)
            {
                _signal.get().unsubscribe(_subscriptionId);
                _subscriptionId = 0;
            }
        }

        void store(Args... args)
        {
            detail::store_signal_awaitable_result<Result>(_state, std::forward<Args>(args)...);
        }

        std::reference_wrapper<ISignal<Args...>> _signal;
        Id _subscriptionId = 0;
        std::coroutine_handle<> _coroutine = nullptr;
        detail::signal_awaitable_state<Result> _state;
    };

    template <class... Args>
    SignalAwaitable<Args...> wait_signal(ISignal<Args...>& signal)
    {
        return SignalAwaitable<Args...>(signal);
    }
}
