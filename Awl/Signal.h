/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/EquatableFunction.h"

#include <algorithm>
#include <cstddef>
#include <concepts>
#include <memory>
#include <utility>
#include <vector>

namespace awl
{
    template <class... Args>
    class Signal
    {
    public:

        using Slot = equatable_function<void(Args...)>;
        using container_type = std::vector<Slot>;

        void subscribe(Slot slot)
        {
            if (!slot)
            {
                return;
            }

            if (std::find(m_slots.begin(), m_slots.end(), slot) == m_slots.end())
            {
                m_slots.push_back(std::move(slot));
            }
        }

        template <class Object>
        void subscribe(Object* p_object, void (Object::*member)(Args...))
        {
            subscribe(Slot(p_object, member));
        }

        template <class Object>
        void subscribe(const Object* p_object, void (Object::*member)(Args...) const)
        {
            subscribe(Slot(p_object, member));
        }

        template <class Object>
        void subscribe(const std::shared_ptr<Object>& p_object, void (Object::*member)(Args...))
        {
            subscribe(Slot(p_object, member));
        }

        template <class Object>
        void subscribe(const std::shared_ptr<Object>& p_object, void (Object::*member)(Args...) const)
        {
            subscribe(Slot(p_object, member));
        }

        template <class Object>
        void subscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...))
        {
            subscribe(Slot(std::move(p_object), member));
        }

        template <class Object>
        void subscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...) const)
        {
            subscribe(Slot(std::move(p_object), member));
        }

        bool unsubscribe(const Slot& slot)
        {
            const auto it = std::find(m_slots.begin(), m_slots.end(), slot);

            if (it == m_slots.end())
            {
                return false;
            }

            auto last = m_slots.end();
            --last;

            if (it != last)
            {
                std::iter_swap(it, last);
            }

            m_slots.pop_back();
            return true;
        }

        template <class Object>
        bool unsubscribe(Object* p_object, void (Object::*member)(Args...))
        {
            return unsubscribe(Slot(p_object, member));
        }

        template <class Object>
        bool unsubscribe(const Object* p_object, void (Object::*member)(Args...) const)
        {
            return unsubscribe(Slot(p_object, member));
        }

        template <class Object>
        bool unsubscribe(const std::shared_ptr<Object>& p_object, void (Object::*member)(Args...))
        {
            return unsubscribe(Slot(p_object, member));
        }

        template <class Object>
        bool unsubscribe(const std::shared_ptr<Object>& p_object, void (Object::*member)(Args...) const)
        {
            return unsubscribe(Slot(p_object, member));
        }

        template <class Object>
        bool unsubscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...))
        {
            return unsubscribe(Slot(std::move(p_object), member));
        }

        template <class Object>
        bool unsubscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...) const)
        {
            return unsubscribe(Slot(std::move(p_object), member));
        }

        template<typename ...Params>
        void emit(const Params&... args) const
            requires (std::invocable<Slot&, const Params&...>)
        {
            std::size_t active_end = m_slots.size();
            std::size_t i = 0;

            while (i < active_end)
            {
                auto guard = m_slots[i].lock();

                if (guard)
                {
                    guard(args...);
                    ++i;
                }
                else
                {
                    --active_end;

                    if (i != active_end)
                    {
                        std::iter_swap(m_slots.begin() + static_cast<std::ptrdiff_t>(i), m_slots.begin() + static_cast<std::ptrdiff_t>(active_end));
                    }
                }
            }

            m_slots.erase(m_slots.begin() + static_cast<std::ptrdiff_t>(active_end), m_slots.end());
        }

        void clear() noexcept
        {
            m_slots.clear();
        }

        bool empty() const noexcept
        {
            return m_slots.empty();
        }

        std::size_t size() const noexcept
        {
            return m_slots.size();
        }

    private:

        mutable container_type m_slots;
    };
}
