/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/ObservableImpl.h"

namespace awl
{
    template <class IObserver, class Enclosing = void>
    class Observable : private details::ObservableImpl<IObserver, Enclosing>
    {
    private:

        using Base = details::ObservableImpl<IObserver, Enclosing>;
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

        //Separating Params and Args prevents ambiguity for const ref parameter types. The method invocation will produce 
        //compiler errors if Args does not match Params.
        template<typename ...Params, typename ... Args>
        void notify(void (IObserver::* func)(Params ...), const Args& ... args)
            requires (std::invocable<decltype(func), IObserver*, const Args&...>)
        {
            Base::notifyImpl([&](ObserverElement* p_observer) { (static_cast<IObserver*>(p_observer)->*func)(args ...); });
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
            return Base::notifyWhileTrueImpl([&](ObserverElement* p_observer) { return (static_cast<IObserver*>(p_observer)->*func)(args ...); });
        }

        friend Enclosing;
    };
}
