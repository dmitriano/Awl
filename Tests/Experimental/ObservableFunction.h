/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Tests/Experimental/ObserverFunction.h"

#include "Awl/Observable.h"

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace awl
{
    // Storing this pointer in a std::function with std::bind, for example, prevents the observers from being movable,
    // so this class is experimental and is not used in production.
    template <class Result, class... Params, class Enclosing>
    class Observable<std::function<Result(Params...)>, Enclosing>
    {
    private:

        using Function = std::function<Result(Params...)>;
        using ObserverElement = Observer<Function>;
        using ObserverList = quick_list<ObserverElement, observer_link>;

    public:

        Observable() = default;

        ~Observable()
        {
            clearObservers();
        }

        Observable(const Observable& other) = delete;

        Observable(Observable&& other) : _observers(std::move(other._observers)) {}

        Observable& operator = (const Observable& other) = delete;

        Observable& operator = (Observable&& other) noexcept
        {
            clearObservers();
            _observers = std::move(other._observers);
            return *this;
        }

        void subscribe(ObserverElement* p_observer)
        {
            _observers.push_back(p_observer);
        }

        void unsubscribe(ObserverElement* p_observer)
        {
            p_observer->unsubscribeSelf();
        }

        bool empty() const
        {
            return _observers.empty();
        }

        auto size() const
        {
            return _observers.size();
        }

    protected:

        template<typename ... Args>
        void notify(const Args& ... args)
            requires (std::invocable<Function&, const Args&...>)
        {
            notifyLoopImpl([&](ObserverElement* p_observer)
            {
                p_observer->function()(args ...);
                return true;
            });
        }

        template<typename ... Args>
        bool notifyWhile(const Args& ... args)
            requires (
                std::is_convertible_v<Result, bool> &&
                std::invocable<Function&, const Args&...>
            )
        {
            return notifyLoopImpl([&](ObserverElement* p_observer) { return p_observer->function()(args ...); });
        }

        template<typename ... Args>
        bool notifyUntil(const Args& ... args)
            requires (
                std::is_convertible_v<Result, bool> &&
                std::invocable<Function&, const Args&...>
            )
        {
            return !notifyLoopImpl([&](ObserverElement* p_observer)
            {
                return !static_cast<bool>(p_observer->function()(args ...));
            });
        }

    private:

        friend Enclosing;

        template <class Callable>
        bool notifyLoopImpl(Callable&& call)
            requires (
                std::invocable<Callable&, ObserverElement*> &&
                std::convertible_to<std::invoke_result_t<Callable&, ObserverElement*>, bool>
            )
        {
            for (typename ObserverList::iterator i = _observers.begin(); i != _observers.end(); )
            {
                ObserverElement* p_observer = *(i++);

                if (!static_cast<bool>(call(p_observer)))
                {
                    return false;
                }
            }

            return true;
        }

        void clearObservers()
        {
            while (!_observers.empty())
            {
                _observers.pop_front();
            }
        }

        ObserverList _observers;
    };
}
