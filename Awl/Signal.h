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
    // A Slot is a class like this:
    // template <class Result, class... Params>
    // class Slot
    // {
    // public:

    //     virtual Result operator()(Params... args) = 0;
    // };

    template <class Slot, class Enclosing = void>
    class Signal : private details::ObservableImpl<Slot, Enclosing>
    {
    private:

        using IObserver = Slot;
        using Base = details::ObservableImpl<IObserver, Enclosing>;
        using ObserverElement = typename Base::ObserverElement;

    public:

        Signal() = default;
        ~Signal() = default;

        Signal(const Signal& other) = delete;
        Signal(Signal&& other) = default;

        Signal& operator = (const Signal& other) = delete;
        Signal& operator = (Signal&& other) noexcept = default;

        using Base::subscribe;
        using Base::unsubscribe;
        using Base::empty;
        using Base::size;

    protected:

        template<typename ...Args>
        void emit(const Args&... args)
            requires (std::invocable<IObserver&, const Args&...>)
        {
            Base::notifyImpl([&](ObserverElement* p_observer) { (*static_cast<IObserver*>(p_observer))(args...); });
        }

        template<typename ...Args>
        bool emitWhileTrue(const Args&... args)
            requires (
                std::invocable<IObserver&, const Args&...> &&
                std::is_convertible_v<std::invoke_result_t<IObserver&, const Args&...>, bool>
            )
        {
            return Base::notifyWhileTrueImpl([&](ObserverElement* p_observer) { return (*static_cast<IObserver*>(p_observer))(args...); });
        }

        friend Enclosing;
    };
}
