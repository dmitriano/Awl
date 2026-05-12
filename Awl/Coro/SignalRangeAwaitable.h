#pragma once

#include "Awl/INotifySetChanged.h"
#include "Awl/RangeUtil.h"
#include "Awl/Signal.h"

#include <algorithm>
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

        template <class Object, auto get_signal, class SignalType>
        class BasicSignalRangeAwaitable;

        template <class Object, auto get_signal, class... Args>
        class BasicSignalRangeAwaitable<Object, get_signal, ISignal<Args...>>
        {
        private:

            using Result = detail::signal_range_awaitable_result_t<Object, Args...>;
            using ObjectPtr = std::shared_ptr<Object>;
            using SetObserverBase = Observer<INotifySetChanged<ObjectPtr>>;

            struct State
            {
                std::optional<Result> result;
            };

            struct Subscription
            {
                ObjectPtr object;
                Id id = 0;
            };

            class SetObserver : public SetObserverBase
            {
            public:

                explicit SetObserver(BasicSignalRangeAwaitable& owner) :
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

                BasicSignalRangeAwaitable& _owner;
            };

        public:

            template <awl::input_range_over<ObjectPtr> R>
            explicit BasicSignalRangeAwaitable(R&& objects)
            {
                if constexpr (awl::observable_shared_ptr_set<R>)
                {
                    static_assert(
                        std::is_lvalue_reference_v<R>,
                        "An observable set passed to wait_signal must outlive the awaiting coroutine.");

                    auto* p_objects = std::addressof(objects);

                    _subscribeSet = [p_objects](SetObserver* p_observer)
                    {
                        p_objects->subscribe(p_observer);
                    };
                }

                for (auto&& object : objects)
                {
                    addObject(std::forward<decltype(object)>(object));
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
                assert(observesSet() || !_subscriptions.empty());

                _coroutine = h;

                if (observesSet())
                {
                    _subscribeSet(&_setObserver);
                }

                for (Subscription& subscription : _subscriptions)
                {
                    subscribeObject(subscription);
                }
            }

            Result await_resume()
            {
                return std::move(*_state.result);
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
                return _subscriptions.back();
            }

            void subscribeObject(Subscription& subscription)
            {
                if (subscription.id == 0)
                {
                    ISignal<Args...>& signal = getSignal(*subscription.object);
                    ObjectPtr object = subscription.object;

                    subscription.id = signal.subscribe(std::function<void(Args...)>(
                        [this, object = std::move(object)](Args... args)
                        {
                            resume(object, std::forward<Args>(args)...);
                        }));
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
                assert(!_done);

                Subscription& subscription = addObject(object);

                if (_coroutine != nullptr)
                {
                    subscribeObject(subscription);
                }
            }

            void onRemoving(const ObjectPtr& object)
            {
                assert(!_done);

                removeObject(object);
            }

            void onClearing()
            {
                assert(!_done);

                unsubscribeObjects();
                _subscriptions.clear();
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
                _setObserver.unsubscribeSafe();
                unsubscribeObjects();
                _subscriptions.clear();
            }

            std::vector<Subscription> _subscriptions;
            SetObserver _setObserver{ *this };
            std::function<void(SetObserver*)> _subscribeSet;
            bool _done = false;
            std::coroutine_handle<> _coroutine = nullptr;
            State _state;
        };

        template <class Object, auto get_signal, class... Args>
        class BasicSignalRangeAwaitable<Object, get_signal, Source<Args...>> :
            public BasicSignalRangeAwaitable<Object, get_signal, ISignal<Args...>>
        {
        private:

            using Base = BasicSignalRangeAwaitable<Object, get_signal, ISignal<Args...>>;

        public:

            using Base::Base;
        };
    }

    template <class Object, auto get_signal>
    using SignalRangeAwaitable = detail::BasicSignalRangeAwaitable<
        Object,
        get_signal,
        std::remove_cvref_t<std::invoke_result_t<decltype(get_signal), Object&>>>;

    template <auto get_signal, awl::input_shared_ptr_range R>
    SignalRangeAwaitable<typename std::ranges::range_value_t<R>::element_type, get_signal> wait_signal(R&& objects)
    {
        using Object = typename std::ranges::range_value_t<R>::element_type;

        return SignalRangeAwaitable<Object, get_signal>(std::forward<R>(objects));
    }
}
