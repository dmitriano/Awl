#pragma once

#include "Awl/QuickList.h"

namespace awl
{
    template <class IObserver>
    class Observer : public IObserver, public quick_link
    {
    private:

        typedef quick_link Base;

    public:

        bool IsSubscribed() const
        {
            return Base::included();
        }

        void UnsubscribeSelf()
        {
            Base::exclude();
        }

        ~Observer()
        {
            if (IsSubscribed())
            {
                UnsubscribeSelf();
            }
        }
    };

    template <class IObserver>
    class Observable
    {
    public:

        typedef Observer<IObserver> OBSERVER;

        Observable()
        {
        }

        ~Observable()
        {
            //If the observable is deleted before its observers,
            //we remove them from the list, otherwise they will think that they are included and
            //their destructors will delete them from already destroyed list.
            while (!Observers.empty())
            {
                Observers.pop_front();
            }
        }

        Observable(const Observable& other) = delete;

        Observable(Observable&& other) : Observers(std::move(other.Observers))
        {
        }

        Observable& operator = (const Observable& other) = delete;

        Observable& operator = (Observable&& other)
        {
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

    private:

        typedef quick_list<OBSERVER> OBSERVER_LIST;

        OBSERVER_LIST Observers;
    };
}
