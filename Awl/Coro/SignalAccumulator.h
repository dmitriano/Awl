#pragma once

#include "Awl/ISignal.h"

#include <cassert>
#include <coroutine>
#include <deque>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace awl
{
    template <class... Args>
    class SignalAccumulator;

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

    template <class... Args>
    class SignalAccumulator
    {
    private:

        using Result = detail::signal_accumulator_result_t<Args...>;
        using StoredResult = detail::signal_accumulator_storage_t<Result>;

        class Awaitable;

        struct State
        {
            std::deque<StoredResult> results;
            Awaitable* waiter = nullptr;
        };

    public:

        explicit SignalAccumulator(ISignal<Args...>& signal) :
            _signal(signal),
            _state(std::make_shared<State>())
        {
            _subscriptionId = _signal.subscribe(
                [this](Args... args)
                {
                    resume(std::forward<Args>(args)...);
                });
        }

        SignalAccumulator(const SignalAccumulator&) = delete;

        SignalAccumulator(SignalAccumulator&&) = delete;

        SignalAccumulator& operator = (const SignalAccumulator&) = delete;

        SignalAccumulator& operator = (SignalAccumulator&&) = delete;

        ~SignalAccumulator()
        {
            unsubscribe();
        }

        Awaitable wait()
        {
            return Awaitable(_state);
        }

    private:

        class Awaitable
        {
        public:

            explicit Awaitable(std::shared_ptr<State> state) :
                _state(std::move(state))
            {}

            Awaitable(const Awaitable&) = delete;

            Awaitable(Awaitable&& other) noexcept :
                _state(std::move(other._state)),
                _coroutine(std::exchange(other._coroutine, nullptr))
            {
                if (_state && _state->waiter == &other)
                {
                    _state->waiter = this;
                }
            }

            Awaitable& operator = (const Awaitable&) = delete;

            Awaitable& operator = (Awaitable&& other) noexcept
            {
                if (this != &other)
                {
                    unsubscribe();

                    _state = std::move(other._state);
                    _coroutine = std::exchange(other._coroutine, nullptr);

                    if (_state && _state->waiter == &other)
                    {
                        _state->waiter = this;
                    }
                }

                return *this;
            }

            ~Awaitable()
            {
                unsubscribe();
            }

            bool await_ready() const noexcept
            {
                return !_state->results.empty();
            }

            bool await_suspend(std::coroutine_handle<> h)
            {
                assert(_coroutine == nullptr);

                _coroutine = h;

                if (!_state->results.empty())
                {
                    _coroutine = nullptr;

                    return false;
                }

                assert(_state->waiter == nullptr);

                _state->waiter = this;

                return true;
            }

            decltype(auto) await_resume()
            {
                unsubscribe();

                assert(!_state->results.empty());

                if constexpr (std::is_void_v<Result>)
                {
                    _state->results.pop_front();

                    return;
                }
                else
                {
                    Result result = std::move(_state->results.front());
                    _state->results.pop_front();

                    return result;
                }
            }

            void resume()
            {
                if (std::coroutine_handle<> coroutine = std::exchange(_coroutine, nullptr))
                {
                    coroutine.resume();
                }
            }

        private:

            void unsubscribe()
            {
                if (_coroutine != nullptr)
                {
                    assert(_state);
                    assert(_state->waiter == this);

                    _state->waiter = nullptr;
                    _coroutine = nullptr;
                }
            }

            std::shared_ptr<State> _state;
            std::coroutine_handle<> _coroutine = nullptr;
        };

        void push(Args... args)
        {
            if constexpr (std::is_void_v<Result>)
            {
                static_cast<void>(sizeof...(args));

                _state->results.emplace_back();
            }
            else
            {
                _state->results.emplace_back(std::forward<Args>(args)...);
            }
        }

        void resume(Args... args)
        {
            push(std::forward<Args>(args)...);

            if (Awaitable* waiter = std::exchange(_state->waiter, nullptr))
            {
                waiter->resume();
            }
        }

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
        std::shared_ptr<State> _state;
    };

    template <class... Args>
    SignalAccumulator<Args...> accumulate_signal(ISignal<Args...>& signal)
    {
        return SignalAccumulator<Args...>(signal);
    }
}
