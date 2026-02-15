/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Observer.h"

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

            ObserverList& observers()
            {
                return m_observers;
            }

            const ObserverList& observers() const
            {
                return m_observers;
            }

        private:

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
    class Observable : public details::ObservableImpl<IObserver, Enclosing>
    {
    private:

        using Base = details::ObservableImpl<IObserver, Enclosing>;
        using ObserverList = typename Base::ObserverList;

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
            requires (sizeof...(Params) == sizeof...(Args) && (std::is_convertible_v<Args, Params> && ...))
        {
            ObserverList& observer_list = Base::observers();

            for (typename ObserverList::iterator i = observer_list.begin(); i != observer_list.end(); )
            {
                //p_observer can delete itself or unsubscribe while iterating over the list so we use postfix ++
                IObserver * p_observer = *(i++);

                (p_observer->*func)(args ...);
            }
        }

        // It is not clear enough if we really need const notify methods like this:
        // template<typename ...Params, typename ... Args>
        // void notify(void (IObserver::*func)(Params ...) const, const Args& ... args) const

        template<typename TResult, typename ...Params, typename ... Args>
        bool notifyWhileTrue(TResult (IObserver::* func)(Params ...), const Args& ... args)
            requires (
                std::is_convertible_v<TResult, bool> &&
                sizeof...(Params) == sizeof...(Args) &&
                (std::is_convertible_v<Args, Params> && ...)
            )
        {
            ObserverList& observer_list = Base::observers();

            for (typename ObserverList::iterator i = observer_list.begin(); i != observer_list.end(); )
            {
                //p_observer can delete itself or unsubscribe while iterating over the list so we use postfix ++
                IObserver* p_observer = *(i++);

                if (!static_cast<bool>((p_observer->*func)(args ...)))
                {
                    return false;
                }
            }

            return true;
        }

    private:
        friend Enclosing;
    };

    template <class TResult, class... Params, class Enclosing>
    class Observable<std::function<TResult(Params...)>, Enclosing> :
        public details::ObservableImpl<std::function<TResult(Params...)>, Enclosing>
    {
    private:

        using Base = details::ObservableImpl<std::function<TResult(Params...)>, Enclosing>;
        using FunctionObserver = std::function<TResult(Params...)>;
        using ObserverList = typename Base::ObserverList;

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
        {
            ObserverList& observer_list = Base::observers();

            for (typename ObserverList::iterator i = observer_list.begin(); i != observer_list.end(); )
            {
                //p_observer can delete itself or unsubscribe while iterating over the list so we use postfix ++
                FunctionObserver* p_observer = *(i++);

                (*p_observer)(args ...);
            }
        }

        template<typename ... Args>
        bool notifyWhileTrue(const Args& ... args)
            requires std::is_convertible_v<TResult, bool>
        {
            ObserverList& observer_list = Base::observers();

            for (typename ObserverList::iterator i = observer_list.begin(); i != observer_list.end(); )
            {
                //p_observer can delete itself or unsubscribe while iterating over the list so we use postfix ++
                FunctionObserver* p_observer = *(i++);

                if (!static_cast<bool>((*p_observer)(args ...)))
                {
                    return false;
                }
            }

            return true;
        }

    private:
        friend Enclosing;
    };
}
