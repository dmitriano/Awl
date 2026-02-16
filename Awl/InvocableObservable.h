/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/GeneralObservable.h"

#include <concepts>
#include <type_traits>

namespace awl
{
    namespace details
    {
        template <class IObserver, class Result, class Enclosing, class... Params>
            requires std::invocable<IObserver&, Params...>
        class Observable : private ObservableImpl<IObserver, Enclosing>
        {
        private:

            using Base = ObservableImpl<IObserver, Enclosing>;
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
                    std::is_convertible_v<Result, bool> &&
                    std::invocable<IObserver&, const Args&...>
                )
            {
                return Base::notifyWhileTrueImpl([&](ObserverElement* p_observer) { return (*static_cast<IObserver*>(p_observer))(args ...); });
            }

            friend Enclosing;
        };
    }
}
