/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Coro/Task.h"
#include "Awl/Observer.h"

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace awl
{
    template <class IObserver, class Enclosing = void>
    class Observable
    {
    private:

        using ObserverElement = Observer<IObserver>;
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

        //Separating Params and Args prevents ambiguity for const ref parameter types. The method invocation will produce 
        //compiler errors if Args does not match Params.
        template<typename ...Params, typename ... Args>
        void notify(void (IObserver::* func)(Params ...), const Args& ... args)
            requires (std::invocable<decltype(func), IObserver*, const Args&...>)
        {
            forEach([&](IObserver& observer)
            {
                (observer.*func)(args ...);
            });
        }

        // It is not clear enough if we really need const notify methods like this:
        // template<typename ...Params, typename ... Args>
        // void notify(void (IObserver::*func)(Params ...) const, const Args& ... args) const

        template<typename Result, typename ...Params, typename ... Args>
        bool notifyWhile(Result(IObserver::* func)(Params ...), const Args& ... args)
            requires (
                std::is_convertible_v<Result, bool>&&
                    std::invocable<decltype(func), IObserver*, const Args&...>
            )
        {
            return notifyWhileImpl(func, args ...);
        }

        template<typename Result, typename ...Params, typename ... Args>
        bool notifyWhile(Result(IObserver::* func)(Params ...) const, const Args& ... args)
            requires (
                std::is_convertible_v<Result, bool>&&
                    std::invocable<decltype(func), IObserver*, const Args&...>
            )
        {
            return notifyWhileImpl(func, args ...);
        }

        template<typename Result, typename ...Params, typename ... Args>
        bool notifyUntil(Result(IObserver::* func)(Params ...), const Args& ... args)
            requires (
                std::is_convertible_v<Result, bool>&&
                    std::invocable<decltype(func), IObserver*, const Args&...>
            )
        {
            return notifyUntilImpl(func, args ...);
        }

        template<typename Result, typename ...Params, typename ... Args>
        bool notifyUntil(Result(IObserver::* func)(Params ...) const, const Args& ... args)
            requires (
                std::is_convertible_v<Result, bool>&&
                    std::invocable<decltype(func), IObserver*, const Args&...>
            )
        {
            return notifyUntilImpl(func, args ...);
        }

        template<typename ...Params, typename ... Args>
        Task<void> notifyAsync(Task<void> (IObserver::* func)(Params ...), Args ... args)
            requires (std::invocable<decltype(func), IObserver*, Args&...>)
        {
            for (typename ObserverList::iterator i = _observers.begin(); i != _observers.end(); )
            {
                ObserverElement* p_observer = *(i++);

                co_await (static_cast<IObserver*>(p_observer)->*func)(args ...);
            }
        }

    private:

        friend Enclosing;

        template <class Func, typename ...Args>
        bool notifyWhileImpl(Func func, const Args& ... args)
        {
            return forEachWhile([&](IObserver& observer)
            {
                return std::invoke(func, observer, args ...);
            });
        }

        template <class Func, typename ...Args>
        bool notifyUntilImpl(Func func, const Args& ... args)
        {
            return !forEachWhile([&](IObserver& observer)
            {
                return !static_cast<bool>(std::invoke(func, observer, args ...));
            });
        }

        // Not const: observers may unsubscribe or destroy themselves during notification.
        template <class Callable>
        void forEach(Callable&& call)
            requires (
                std::invocable<Callable&, IObserver&> &&
                std::same_as<std::invoke_result_t<Callable&, IObserver&>, void>
            )
        {
            forEachWhile([&](IObserver& observer)
            {
                call(observer);
                return true;
            });
        }

        template <class Callable>
        bool forEachWhile(Callable&& call)
            requires (
                std::invocable<Callable&, IObserver&> &&
                std::convertible_to<std::invoke_result_t<Callable&, IObserver&>, bool>
            )
        {
            for (typename ObserverList::iterator i = _observers.begin(); i != _observers.end(); )
            {
                //p_observer can delete itself or unsubscribe while iterating over the list so we use postfix ++
                ObserverElement* p_observer = *(i++);

                if (!static_cast<bool>(call(*static_cast<IObserver*>(p_observer))))
                {
                    return false;
                }
            }

            return true;
        }

        //If the observable is deleted before its observers,
        //we remove them from the list, otherwise they will think that they are included and
        //their destructors will delete them from already destroyed list.
        //So we can't use _observers.clear() here because it only clears list's head.
        void clearObservers()
        {
            while (!_observers.empty())
            {
                _observers.pop_front();
            }
        }

        // Do not make this mutable: const notification could create a false impression of thread safety.
        ObserverList _observers;
    };
}
