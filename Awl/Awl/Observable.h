#pragma once

#include "Awl/QuickList.h"

namespace awl
{
    template <class IObserver>
    class Observer : public IObserver, public quick_link< Observer<IObserver> >
    {
    public:

        bool IsSubscribed() const
        {
            return included();
        }
        
        void UnsubscribeSelf()
        {
            exclude();
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

        void Subscribe(OBSERVER * p_observer)
	    {
		    Observers.push_back(p_observer);
	    }

        void Unsubscribe(OBSERVER * p_observer)
        {
            p_observer->UnsubscribeSelf();
        }

    protected:
        
		template<typename ... Args>
		void Notify(void (IObserver::*func)(Args ...), Args ... args)
		{
			for (OBSERVER_LIST::iterator i = Observers.begin(); i != Observers.end(); )
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