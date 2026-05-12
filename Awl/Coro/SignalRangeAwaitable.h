#pragma once

#include "Awl/Signal.h"

#include <cassert>
#include <coroutine>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace awl
{
    namespace detail
    {
        template <class Object, class... Args>
        struct signal_range_awaitable_result
        {
            using type = std::tuple<std::shared_ptr<Object>, std::decay_t<Args>...>;
        };

        template <class Object>
        struct signal_range_awaitable_result<Object>
        {
            using type = std::shared_ptr<Object>;
        };

        template <class Object, class... Args>
        using signal_range_awaitable_result_t = typename signal_range_awaitable_result<Object, Args...>::type;

        template <class Result, class Object, class State, class... Args>
        void store_signal_range_awaitable_result(State& state, std::shared_ptr<Object> sender, Args&&... args)
        {
            if constexpr (sizeof...(Args) == 0)
            {
                state.result = std::move(sender);
            }
            else
            {
                state.result.emplace(std::move(sender), std::forward<Args>(args)...);
            }
        }

        template <class T>
        struct shared_ptr_traits;

        template <class T>
        struct shared_ptr_traits<std::shared_ptr<T>>
        {
            using element_type = T;
        };

        template <class T>
        concept shared_ptr_reference = requires
        {
            typename shared_ptr_traits<std::remove_cvref_t<T>>::element_type;
        };

        template <class R>
        concept shared_ptr_range = std::ranges::input_range<R> &&
            shared_ptr_reference<std::ranges::range_reference_t<R>>;

        template <class R>
        using shared_ptr_range_element_t = typename shared_ptr_traits<
            std::remove_cvref_t<std::ranges::range_reference_t<R>>>::element_type;

        template <class Object, auto get_signal, class SignalType>
        class BasicSignalRangeAwaitable;

        template <class Object, auto get_signal, class... Args>
        class BasicSignalRangeAwaitable<Object, get_signal, Signal<Args...>>
        {
        private:

            using Result = detail::signal_range_awaitable_result_t<Object, Args...>;

            struct State
            {
                std::optional<Result> result;
            };

        public:

            template <std::ranges::input_range R>
                requires detail::shared_ptr_range<R> &&
                    std::same_as<detail::shared_ptr_range_element_t<R>, Object>
            explicit BasicSignalRangeAwaitable(R&& objects)
            {
                for (auto&& object : objects)
                {
                    _objects.push_back(std::forward<decltype(object)>(object));
                }
            }

            BasicSignalRangeAwaitable(const BasicSignalRangeAwaitable&) = delete;

            BasicSignalRangeAwaitable(BasicSignalRangeAwaitable&&) = delete;

            BasicSignalRangeAwaitable& operator = (const BasicSignalRangeAwaitable&) = delete;

            BasicSignalRangeAwaitable& operator = (BasicSignalRangeAwaitable&&) = delete;

            ~BasicSignalRangeAwaitable()
            {
                unsubscribe();
            }

            bool await_ready() const noexcept
            {
                return false;
            }

            void await_suspend(std::coroutine_handle<> h)
            {
                assert(!_objects.empty());

                _coroutine = h;
                _subscriptionIds.reserve(_objects.size());

                for (const std::shared_ptr<Object>& object : _objects)
                {
                    Signal<Args...>& signal = getSignal(*object);

                    _subscriptionIds.push_back(signal.subscribe(std::function<void(Args...)>(
                        [this, object](Args... args)
                        {
                            resume(object, std::forward<Args>(args)...);
                        })));
                }
            }

            Result await_resume()
            {
                return std::move(*_state.result);
            }

        private:

            static Signal<Args...>& getSignal(Object& object)
            {
                return std::invoke(get_signal, object);
            }

            void resume(std::shared_ptr<Object> sender, Args... args)
            {
                if (!_done)
                {
                    _done = true;
                    detail::store_signal_range_awaitable_result<Result>(_state, std::move(sender), std::forward<Args>(args)...);
                    unsubscribe();
                    _coroutine.resume();
                }
            }

            void unsubscribe()
            {
                for (std::size_t i = 0; i != _subscriptionIds.size(); ++i)
                {
                    Id& id = _subscriptionIds[i];

                    if (id != 0)
                    {
                        Signal<Args...>& signal = getSignal(*_objects[i]);
                        signal.unsubscribe(id);
                        id = 0;
                    }
                }
            }

            std::vector<std::shared_ptr<Object>> _objects;
            std::vector<Id> _subscriptionIds;
            bool _done = false;
            std::coroutine_handle<> _coroutine = nullptr;
            State _state;
        };
    }

    template <class Object, auto get_signal>
    using SignalRangeAwaitable = detail::BasicSignalRangeAwaitable<
        Object,
        get_signal,
        std::remove_cvref_t<std::invoke_result_t<decltype(get_signal), Object&>>>;

    template <auto get_signal, std::ranges::input_range R>
        requires detail::shared_ptr_range<R>
    SignalRangeAwaitable<detail::shared_ptr_range_element_t<R>, get_signal> wait_signal(R&& objects)
    {
        using Object = detail::shared_ptr_range_element_t<R>;

        return SignalRangeAwaitable<Object, get_signal>(std::forward<R>(objects));
    }
}
