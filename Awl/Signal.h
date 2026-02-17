/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/EquatableFunction.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace awl
{
    template <class Signature>
    class Signal;

    template <class Result, class... Args>
    class Signal<Result(Args...)>
    {
    public:

        using Slot = equatable_function<Result(Args...)>;
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
        void subscribe(Object* p_object, Result (Object::*member)(Args...))
        {
            subscribe(Slot(p_object, member));
        }

        template <class Object>
        void subscribe(const Object* p_object, Result (Object::*member)(Args...) const)
        {
            subscribe(Slot(p_object, member));
        }

        template <class Object>
        void subscribe(Object& object, Result (Object::*member)(Args...))
        {
            subscribe(Slot(object, member));
        }

        template <class Object>
        void subscribe(const Object& object, Result (Object::*member)(Args...) const)
        {
            subscribe(Slot(object, member));
        }

        bool unsubscribe(const Slot& slot)
        {
            const auto old_size = m_slots.size();
            m_slots.erase(std::remove(m_slots.begin(), m_slots.end(), slot), m_slots.end());
            return m_slots.size() != old_size;
        }

        template <class Object>
        bool unsubscribe(Object* p_object, Result (Object::*member)(Args...))
        {
            return unsubscribe(Slot(p_object, member));
        }

        template <class Object>
        bool unsubscribe(const Object* p_object, Result (Object::*member)(Args...) const)
        {
            return unsubscribe(Slot(p_object, member));
        }

        template <class Object>
        bool unsubscribe(Object& object, Result (Object::*member)(Args...))
        {
            return unsubscribe(Slot(object, member));
        }

        template <class Object>
        bool unsubscribe(const Object& object, Result (Object::*member)(Args...) const)
        {
            return unsubscribe(Slot(object, member));
        }

        void emit(Args... args) const
        {
            for (const Slot& slot : m_slots)
            {
                static_cast<void>(slot(std::forward<Args>(args)...));
            }
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

        container_type m_slots;
    };
}
