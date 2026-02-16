/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Tests/Experimental/ObserverFunction.h"
#include "Awl/Observable.h"

#include <functional>

namespace awl
{
    template <class Result, class... Params, class Enclosing>
    class Observable<std::function<Result(Params...)>, Enclosing> :
        private details::ObservableImpl<std::function<Result(Params...)>, Enclosing>
    {
    private:

        using Function = std::function<Result(Params...)>;
        using Base = details::ObservableImpl<Function, Enclosing>;
        using ObserverElement = typename Base::ObserverElement;

    public:

        Observable() = default;
        ~Observable() = default;

        Observable(const Observable& other) = delete;
        Observable(Observable&& other) = default;

        Observable& operator = (const Observable& other) = delete;
        Observable& operator = (Observable&& other) noexcept = default;

        using Base::subscribe;
        using Base::unsubscribe;
        using Base::empty;
        using Base::size;

    protected:

        template<typename ... Args>
        void notify(const Args& ... args)
            requires (std::invocable<Function&, const Args&...>)
        {
            Base::notifyImpl([&](ObserverElement* p_observer) { p_observer->function()(args ...); });
        }

        template<typename ... Args>
        bool notifyWhileTrue(const Args& ... args)
            requires (
                std::is_convertible_v<Result, bool> &&
                std::invocable<Function&, const Args&...>
            )
        {
            return Base::notifyWhileTrueImpl([&](ObserverElement* p_observer) { return p_observer->function()(args ...); });
        }

        friend Enclosing;
    };
}
