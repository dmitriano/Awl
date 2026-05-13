#pragma once

#include "Awl/INotifySetChanged.h"
#include "Awl/RangeUtil.h"
#include "Awl/Signal.h"

#include <algorithm>
#include <cassert>
#include <coroutine>
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
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
        void push_signal_range_awaitable_result(State& state, std::shared_ptr<Object> sender, Args&&... args)
        {
            if constexpr (sizeof...(Args) == 0)
            {
                state.results.push_back(std::move(sender));
            }
            else
            {
                state.results.emplace_back(std::move(sender), std::forward<Args>(args)...);
            }
        }

        template <class Object, auto get_signal, class SignalType>
        class BasicSignalRangeAccumulator;

        template <class Object, auto get_signal, class... Args>
        class BasicSignalRangeAccumulator<Object, get_signal, ISignal<Args...>>
        {
        private:

            using Result = detail::signal_range_awaitable_result_t<Object, Args...>;
            using ObjectPtr = std::shared_ptr<Object>;
            using SetObserverBase = Observer<INotifySetChanged<ObjectPtr>>;

            class Awaitable;

            struct State
            {
                std::deque<Result> results;
                Awaitable* waiter = nullptr;
            };

            struct Subscription
            {
                ObjectPtr object;
                Id id = 0;
            };

            class SetObserver : public SetObserverBase
            {
            public:

                explicit SetObserver(BasicSignalRangeAccumulator& owner) :
                    _owner(owner)
                {}

                void onAdded(const ObjectPtr& object) override
                {
                    _owner.onAdded(object);
                }

                void onRemoving(const ObjectPtr& object) override
                {
                    _owner.onRemoving(object);
                }

                void onClearing() override
                {
                    _owner.onClearing();
                }

            private:

                BasicSignalRangeAccumulator& _owner;
            };

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

                Result await_resume()
                {
                    unsubscribe();

                    assert(!_state->results.empty());

                    Result result = std::move(_state->results.front());
                    _state->results.pop_front();

                    return result;
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

        public:

            template <awl::input_range_over<ObjectPtr> R>
            explicit BasicSignalRangeAccumulator(R&& objects) :
                _state(std::make_shared<State>())
            {
                if constexpr (awl::observable_shared_ptr_set<R>)
                {
                    static_assert(
                        std::is_lvalue_reference_v<R>,
                        "An observable set passed to accumulate_signal must outlive the accumulator.");

                    auto* p_objects = std::addressof(objects);

                    _subscribeSet = [p_objects](SetObserver* p_observer)
                    {
                        p_objects->subscribe(p_observer);
                    };

                    _subscribeSet(&_setObserver);
                }

                for (auto&& object : objects)
                {
                    addObject(std::forward<decltype(object)>(object));
                }
            }

            BasicSignalRangeAccumulator(const BasicSignalRangeAccumulator&) = delete;

            BasicSignalRangeAccumulator(BasicSignalRangeAccumulator&&) = delete;

            BasicSignalRangeAccumulator& operator = (const BasicSignalRangeAccumulator&) = delete;

            BasicSignalRangeAccumulator& operator = (BasicSignalRangeAccumulator&&) = delete;

            ~BasicSignalRangeAccumulator()
            {
                unsubscribe();
            }

            Awaitable wait()
            {
                assert(observesSet() || !_subscriptions.empty());

                return Awaitable(_state);
            }

        private:

            static ISignal<Args...>& getSignal(Object& object)
            {
                return std::invoke(get_signal, object);
            }

            bool observesSet() const
            {
                return static_cast<bool>(_subscribeSet);
            }

            Subscription& addObject(ObjectPtr object)
            {
                _subscriptions.push_back(Subscription{ std::move(object) });
                Subscription& subscription = _subscriptions.back();
                subscribeObject(subscription);

                return subscription;
            }

            void subscribeObject(Subscription& subscription)
            {
                if (subscription.id == 0)
                {
                    ISignal<Args...>& signal = getSignal(*subscription.object);
                    ObjectPtr object = subscription.object;

                    subscription.id = signal.subscribe(
                        [this, object = std::move(object)](Args... args)
                        {
                            resume(object, std::forward<Args>(args)...);
                        });
                }
            }

            void unsubscribeObject(Subscription& subscription)
            {
                if (subscription.id != 0)
                {
                    ISignal<Args...>& signal = getSignal(*subscription.object);
                    signal.unsubscribe(subscription.id);
                    subscription.id = 0;
                }
            }

            void unsubscribeObjects()
            {
                for (Subscription& subscription : _subscriptions)
                {
                    unsubscribeObject(subscription);
                }
            }

            auto findSubscription(const ObjectPtr& object)
            {
                return std::ranges::find(_subscriptions, object, &Subscription::object);
            }

            void removeObject(const ObjectPtr& object)
            {
                auto i = findSubscription(object);

                if (i != _subscriptions.end())
                {
                    unsubscribeObject(*i);
                    _subscriptions.erase(i);
                }
            }

            void onAdded(const ObjectPtr& object)
            {
                addObject(object);
            }

            void onRemoving(const ObjectPtr& object)
            {
                removeObject(object);
            }

            void onClearing()
            {
                unsubscribeObjects();
                _subscriptions.clear();
            }

            void resume(std::shared_ptr<Object> sender, Args... args)
            {
                detail::push_signal_range_awaitable_result<Result>(*_state, std::move(sender), std::forward<Args>(args)...);

                if (Awaitable* waiter = std::exchange(_state->waiter, nullptr))
                {
                    waiter->resume();
                }
            }

            void unsubscribe()
            {
                _setObserver.unsubscribeSafe();
                unsubscribeObjects();
                _subscriptions.clear();
            }

            std::vector<Subscription> _subscriptions;
            SetObserver _setObserver{ *this };
            std::function<void(SetObserver*)> _subscribeSet;
            std::shared_ptr<State> _state;
        };

        template <class Object, auto get_signal, class... Args>
        class BasicSignalRangeAccumulator<Object, get_signal, Source<Args...>> :
            public BasicSignalRangeAccumulator<Object, get_signal, ISignal<Args...>>
        {
        private:

            using Base = BasicSignalRangeAccumulator<Object, get_signal, ISignal<Args...>>;

        public:

            using Base::Base;
        };
    }

    template <class Object, auto get_signal>
    using SignalRangeAccumulator = detail::BasicSignalRangeAccumulator<
        Object,
        get_signal,
        std::remove_cvref_t<std::invoke_result_t<decltype(get_signal), Object&>>>;

    template <auto get_signal, awl::input_shared_ptr_range R>
    SignalRangeAccumulator<typename std::ranges::range_value_t<R>::element_type, get_signal> accumulate_signal(R&& objects)
    {
        using Object = typename std::ranges::range_value_t<R>::element_type;

        return SignalRangeAccumulator<Object, get_signal>(std::forward<R>(objects));
    }
}
