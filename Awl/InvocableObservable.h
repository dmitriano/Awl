/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/ObservableImpl.h"

#include <concepts>
#include <type_traits>

namespace awl
{
    template <class Result, class... Params>
    class Slot
    {
    public:

        virtual Result operator()(Params ... args) = 0;
    };

    template <class Result, class... Params, class Enclosing>
    class Observable<Slot<Result, Params...>, Enclosing> :
        private details::ObservableImpl<Slot<Result, Params...>, Enclosing>
    {
    private:

        using IObserver = Slot<Result, Params...>;
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

        template<typename ... Args>
        void notify(const Args& ... args)
            requires (std::invocable<IObserver&, const Args&...>)
        {
            Base::notifyImpl([&](ObserverElement* p_observer) { (*static_cast<IObserver*>(p_observer))(args ...); });
        }

        template<typename ... Args>
        bool notifyWhileTrue(const Args& ... args)
            requires (
        std::is_convertible_v<Result, bool>&&
            std::invocable<IObserver&, const Args&...>
            )
        {
            return Base::notifyWhileTrueImpl([&](ObserverElement* p_observer) { return (*static_cast<IObserver*>(p_observer))(args ...); });
        }

        friend Enclosing;
    };
}
