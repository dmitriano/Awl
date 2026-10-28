#pragma once

#include "BoostExtras/SignalQueue.h"

#include "Awl/INotifySetChanged.h"
#include "Awl/ISignal.h"
#include "Awl/RangeUtil.h"

#include <algorithm>
#include <cstddef>
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

        template <class Object, auto get_signal, class SignalType>
        class BasicSignalRangeAccumulator;

        template <class Object, auto get_signal, class... Args>
        class BasicSignalRangeAccumulator<Object, get_signal, ISignal<Args...>>
        {
        private:

            using Result = detail::signal_range_awaitable_result_t<Object, Args...>;
            using ObjectPtr = std::shared_ptr<Object>;
            using SetObserverBase = Observer<INotifySetChanged<ObjectPtr>>;

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

        public:

            template <awl::input_range_over<ObjectPtr> R>
            BasicSignalRangeAccumulator(const boost::asio::any_io_executor executor, R&& objects) :
                _queue(executor)
            {
                if constexpr (awl::observable_shared_ptr_set<R>)
                {
                    static_assert(
                        std::is_lvalue_reference_v<R>,
                        "An observable set passed to accumulate_signal must outlive the accumulator.");

                    objects.subscribe(&_setObserver);
                }

                try
                {
                    for (auto&& object : objects)
                    {
                        addObject(std::forward<decltype(object)>(object));
                    }
                }
                catch (...)
                {
                    unsubscribe();

                    throw;
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

            boost::asio::awaitable<Result> asyncWait()
            {
                return _queue.asyncWait();
            }

        private:

            static ISignal<Args...>& getSignal(Object& object)
            {
                return std::invoke(get_signal, object);
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
                if constexpr (sizeof...(Args) == 0)
                {
                    _queue.push(std::move(sender));
                }
                else
                {
                    _queue.push(std::move(sender), std::forward<Args>(args)...);
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
            detail::SignalQueue<Result> _queue;
        };
    }

    // Objects/signals outlive the accumulator; observable sets must be lvalues.
    // Construction, set mutation, signal delivery and destruction require executor's strand.
    // Wait cancellation preserves queued events/subscriptions; FIFO is unbounded.
    template <class Object, auto get_signal>
    using SignalRangeAccumulator = detail::BasicSignalRangeAccumulator<
        Object,
        get_signal,
        std::remove_cvref_t<std::invoke_result_t<decltype(get_signal), Object&>>>;

    template <auto get_signal, awl::input_shared_ptr_range R>
    SignalRangeAccumulator<typename std::ranges::range_value_t<R>::element_type, get_signal> accumulate_signal(const boost::asio::any_io_executor executor, R&& objects)
    {
        using Object = typename std::ranges::range_value_t<R>::element_type;

        return SignalRangeAccumulator<Object, get_signal>(executor, std::forward<R>(objects));
    }
}
