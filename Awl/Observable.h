/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Coro/Task.h"
#include "Awl/Observer.h"

#include <concepts>
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

        Observable(Observable&& other) : m_observers(std::move(other.m_observers)) {}

        Observable& operator = (const Observable& other) = delete;

        Observable& operator = (Observable&& other) noexcept
        {
            clearObservers();
            m_observers = std::move(other.m_observers);
            return *this;
        }

        void subscribe(ObserverElement* p_observer)
        {
            m_observers.push_back(p_observer);
        }

        void unsubscribe(ObserverElement* p_observer)
        {
            p_observer->unsubscribeSelf();
        }

        bool empty() const
        {
            return m_observers.empty();
        }

        auto size() const
        {
            return m_observers.size();
        }

    protected:

        //Separating Params and Args prevents ambiguity for const ref parameter types. The method invocation will produce 
        //compiler errors if Args does not match Params.
        template<typename ...Params, typename ... Args>
        void notify(void (IObserver::* func)(Params ...), const Args& ... args)
            requires (std::invocable<decltype(func), IObserver*, const Args&...>)
        {
            forEach([&](ObserverElement* p_observer)
            {
                (static_cast<IObserver*>(p_observer)->*func)(args ...);
                return true;
            });
        }

        // It is not clear enough if we really need const notify methods like this:
        // template<typename ...Params, typename ... Args>
        // void notify(void (IObserver::*func)(Params ...) const, const Args& ... args) const

        template<typename Result, typename ...Params, typename ... Args>
        bool notifyWhileTrue(Result(IObserver::* func)(Params ...), const Args& ... args)
            requires (
                std::is_convertible_v<Result, bool>&&
                    std::invocable<decltype(func), IObserver*, const Args&...>
            )
        {
            return forEach([&](ObserverElement* p_observer)
            {
                return (static_cast<IObserver*>(p_observer)->*func)(args ...);
            });
        }

        template<typename ...Params, typename ... Args>
        Task<void> notifyAsync(Task<void> (IObserver::* func)(Params ...), Args ... args)
            requires (std::invocable<decltype(func), IObserver*, Args&...>)
        {
            for (typename ObserverList::iterator i = m_observers.begin(); i != m_observers.end(); )
            {
                ObserverElement* p_observer = *(i++);

                co_await (static_cast<IObserver*>(p_observer)->*func)(args ...);
            }
        }

        friend Enclosing;

    private:

        // Not const: observers may unsubscribe or destroy themselves during notification.
        template <class Callable>
        bool forEach(Callable&& call)
            requires (
                std::invocable<Callable&, ObserverElement*> &&
                std::convertible_to<std::invoke_result_t<Callable&, ObserverElement*>, bool>
            )
        {
            for (typename ObserverList::iterator i = m_observers.begin(); i != m_observers.end(); )
            {
                //p_observer can delete itself or unsubscribe while iterating over the list so we use postfix ++
                ObserverElement* p_observer = *(i++);

                if (!static_cast<bool>(call(p_observer)))
                {
                    return false;
                }
            }

            return true;
        }

        //If the observable is deleted before its observers,
        //we remove them from the list, otherwise they will think that they are included and
        //their destructors will delete them from already destroyed list.
        //So we can't use m_observers.clear() here because it only clears list's head.
        void clearObservers()
        {
            while (!m_observers.empty())
            {
                m_observers.pop_front();
            }
        }

        ObserverList m_observers;
    };
}
