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
    // A slot is a class that has operator() with the signature of the signal.
    template <class Slot, class Enclosing = void>
    class SignalOnObservable : private details::ObservableImpl<Slot, Enclosing>
    {
    private:

        using IObserver = Slot;
        using Base = details::ObservableImpl<IObserver, Enclosing>;
        using ObserverElement = typename Base::ObserverElement;

    public:

        SignalOnObservable() = default;
        ~SignalOnObservable() = default;

        SignalOnObservable(const SignalOnObservable& other) = delete;
        SignalOnObservable(SignalOnObservable&& other) = default;

        SignalOnObservable& operator = (const SignalOnObservable& other) = delete;
        SignalOnObservable& operator = (SignalOnObservable&& other) noexcept = default;

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
