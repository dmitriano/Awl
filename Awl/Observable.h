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

        using OBSERVER = Observer<IObserver>;

        Observable()
        {
        }

        ~Observable()
        {
            ClearObservers();
        }

        Observable(const Observable& other) = delete;

        Observable(Observable&& other) : Observers(std::move(other.Observers))
        {
        }

        Observable& operator = (const Observable& other) = delete;

        Observable& operator = (Observable&& other) noexcept
        {
            ClearObservers();
            Observers = std::move(other.Observers);
            return *this;
        }

        void Subscribe(OBSERVER * p_observer)
        {
            Observers.push_back(p_observer);
        }

        void Unsubscribe(OBSERVER * p_observer)
        {
            p_observer->UnsubscribeSelf();
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
        void Notify(void (IObserver::*func)(Params ...), const Args& ... args)
        {
            for (typename OBSERVER_LIST::iterator i = Observers.begin(); i != Observers.end(); )
            {
                //p_observer can delete itself or unsubscribe while iterating over the list so we use postfix ++
                IObserver * p_observer = *(i++);

                (p_observer->*func)(args ...);
            }
        }

        template<typename ...Params, typename ... Args>
        bool NotifyWhileTrue(bool (IObserver::* func)(Params ...), const Args& ... args)
        {
            for (typename OBSERVER_LIST::iterator i = Observers.begin(); i != Observers.end(); )
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
        void ClearObservers()
        {
            while (!Observers.empty())
            {
                Observers.pop_front();
            }
        }
        
        using OBSERVER_LIST = quick_list<OBSERVER, observer_link>;

        OBSERVER_LIST Observers;

        friend Enclosing;
    };
}
