/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/QuickList.h"

namespace awl
{
    AWL_DECLARE_QUICK_LINK(observer_link)

    template <class IObserver>
    class Observer : public IObserver, public observer_link
    {
    public:

        Observer() = default;

        Observer(const Observer& other) = delete;

        Observer(Observer&& other) = default;

        Observer& operator = (const Observer& other) = delete;

        Observer& operator = (Observer&& other) = default;

        ~Observer() = default;

        bool isSubscribed() const
        {
            return observer_link::included();
        }

        void unsubscribeSelf()
        {
            observer_link::exclude();
        }

        void unsubscribeSafe()
        {
            if (isSubscribed())
            {
                unsubscribeSelf();
            }
        }
    };
}
