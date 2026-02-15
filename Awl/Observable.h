/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Observer.h"

namespace awl
{
    template <class IObserver, class Enclosing = void>
    class Observable
    {
    private:

        using ObserverElement = Observer<IObserver>;

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
        void notify(void (IObserver::*func)(Params ...), const Args& ... args)
        {
            for (typename ObserverList::iterator i = m_observers.begin(); i != m_observers.end(); )
            {
                //p_observer can delete itself or unsubscribe while iterating over the list so we use postfix ++
                IObserver * p_observer = *(i++);

                (p_observer->*func)(args ...);
            }
        }

        // It is not clear enough if we really need const notify methods like this:
        // template<typename ...Params, typename ... Args>
        // void notify(void (IObserver::*func)(Params ...) const, const Args& ... args) const

        template<typename ...Params, typename ... Args>
        bool notifyWhileTrue(bool (IObserver::* func)(Params ...), const Args& ... args)
        {
            for (typename ObserverList::iterator i = m_observers.begin(); i != m_observers.end(); )
            {
                //p_observer can delete itself or unsubscribe while iterating over the list so we use postfix ++
                IObserver* p_observer = *(i++);

                if (!(p_observer->*func)(args ...))
                {
                    return false;
                }
            }

            return true;
        }

    private:

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
        
        using ObserverList = quick_list<ObserverElement, observer_link>;

        ObserverList m_observers;

        friend Enclosing;
    };
}
