/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Observer.h"

#include <functional>
#include <utility>

namespace awl
{
    template <class Result, class... Args>
    class Observer<std::function<Result(Args...)>> : public observer_link
    {
    public:

        using Function = std::function<Result(Args...)>;

        Observer() = default;

        explicit Observer(Function function) :
            m_function(std::move(function))
        {
        }

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

        const Function& function() const
        {
            return m_function;
        }

        void setFunction(Function function)
        {
            m_function = std::move(function);
        }

    private:

        Function m_function;
    };
}
