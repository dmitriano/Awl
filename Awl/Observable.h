/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Observer.h"

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace awl
{
    namespace details
    {
        template <class IObserver, class Enclosing>
        class ObservableImpl
        {
        protected:

            using ObserverElement = Observer<IObserver>;
            using ObserverList = quick_list<ObserverElement, observer_link>;

        public:

            ObservableImpl() = default;

            ~ObservableImpl()
            {
                clearObservers();
            }

            ObservableImpl(const ObservableImpl& other) = delete;

            ObservableImpl(ObservableImpl&& other) : m_observers(std::move(other.m_observers)) {}

            ObservableImpl& operator = (const ObservableImpl& other) = delete;

            ObservableImpl& operator = (ObservableImpl&& other) noexcept
            {
                clearObservers();
                m_observers = std::move(other.m_observers);
                return *this;
            }

            void subscribe(ObserverElement* p_observer)
            {
                m_observers.push_back(p_observer);
            }

            void unsubscribe(ObserverElement* p_observer)
            {
                p_observer->unsubscribeSelf();
            }

            bool empty() const
            {
                return m_observers.empty();
            }

            auto size() const
            {
                return m_observers.size();
            }

        protected:

            template <class Callable>
            void notifyImpl(Callable&& call)
                requires std::invocable<Callable&, ObserverElement*>
            {
                notifyLoopImpl([&](ObserverElement* p_observer)
                {
                    call(p_observer);
                    return true;
                });
            }

            template <class Callable>
            bool notifyWhileTrueImpl(Callable&& call)
                requires std::invocable<Callable&, ObserverElement*>
            {
                return notifyLoopImpl([&](ObserverElement* p_observer)
                {
                    return static_cast<bool>(call(p_observer));
                });
            }

        private:

            template <class Callable>
            bool notifyLoopImpl(Callable&& call)
                requires (
                    std::invocable<Callable&, ObserverElement*> &&
                    std::convertible_to<std::invoke_result_t<Callable&, ObserverElement*>, bool>
                )
            {
                for (typename ObserverList::iterator i = m_observers.begin(); i != m_observers.end(); )
                {
                    //p_observer can delete itself or unsubscribe while iterating over the list so we use postfix ++
                    ObserverElement* p_observer = *(i++);

                    if (!static_cast<bool>(call(p_observer)))
                    {
                        return false;
                    }
                }

                return true;
            }

            //If the observable is deleted before its observers,
            //we remove them from the list, otherwise they will think that they are included and
            //their destructors will delete them from already destroyed list.
            //So we can't use m_observers.clear() here because it only clears list's head.
            void clearObservers()
            {
                while (!m_observers.empty())
                {
                    m_observers.pop_front();
                }
            }

            ObserverList m_observers;
        };
    }

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
        void notify(void (IObserver::*func)(Params ...), const Args& ... args)
            requires (std::invocable<decltype(func), IObserver*, const Args&...>)
        {
            Base::notifyImpl([&](ObserverElement* p_observer) { (static_cast<IObserver*>(p_observer)->*func)(args ...); });
        }

        // It is not clear enough if we really need const notify methods like this:
        // template<typename ...Params, typename ... Args>
        // void notify(void (IObserver::*func)(Params ...) const, const Args& ... args) const

        template<typename Result, typename ...Params, typename ... Args>
        bool notifyWhileTrue(Result (IObserver::* func)(Params ...), const Args& ... args)
            requires (
                std::is_convertible_v<Result, bool> &&
                std::invocable<decltype(func), IObserver*, const Args&...>
            )
        {
            return Base::notifyWhileTrueImpl([&](ObserverElement* p_observer) { return (static_cast<IObserver*>(p_observer)->*func)(args ...); });
        }

        friend Enclosing;
    };

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
