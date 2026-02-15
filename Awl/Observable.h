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
    public:

        using ObserverElement = Observer<IObserver>;

        Observable() = default;

        ~Observable()
        {
            clearObservers();
        }

        Observable(const Observable& other) = delete;

        Observable(Observable&& other) : Observers(std::move(other.Observers)) {}

        Observable& operator = (const Observable& other) = delete;

        Observable& operator = (Observable&& other) noexcept
        {
            clearObservers();
            Observers = std::move(other.Observers);
            return *this;
        }

        void subscribe(ObserverElement* p_observer)
        {
            Observers.push_back(p_observer);
        }

        void unsubscribe(ObserverElement* p_observer)
        {
            p_observer->unsubscribeSelf();
        }

        bool empty() const
        {
            return Observers.empty();
        }

        auto size() const
        {
            return Observers.size();
        }

    protected:

        //Separating Params and Args prevents ambiguity for const ref parameter types. The method invocation will produce 
        //compiler errors if Args does not match Params.
        template<typename ...Params, typename ... Args>
        void notify(void (IObserver::*func)(Params ...), const Args& ... args) const
        {
            for (typename ObserverList::const_iterator i = Observers.begin(); i != Observers.end(); )
            {
                //p_observer can delete itself or unsubscribe while iterating over the list so we use postfix ++
                IObserver * p_observer = *(i++);

                (p_observer->*func)(args ...);
            }
        }

        template<typename ...Params, typename ... Args>
        bool notifyWhileTrue(bool (IObserver::* func)(Params ...), const Args& ... args) const
        {
            for (typename ObserverList::const_iterator i = Observers.begin(); i != Observers.end(); )
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
        //So we can't use Observers.clear() here because it only clears list's head.
        void clearObservers()
        {
            while (!Observers.empty())
            {
                Observers.pop_front();
            }
        }
        
        using ObserverList = quick_list<ObserverElement, observer_link>;

        ObserverList Observers;

        friend Enclosing;
    };
}
