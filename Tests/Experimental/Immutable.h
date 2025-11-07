/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cassert>
#include <stdexcept>

namespace awl
{
    template <class T>
    class immutable
    {
    public:
        
        template <class... Args>
        constexpr immutable(Args&&... args) : m_val(std::forward<Args>(args)...) {}

        constexpr immutable(const immutable& other) : m_val(other.m_val) {}

        constexpr immutable(immutable&& other) noexcept : m_val(other.m_val) {}

        immutable& operator=(const immutable& other)
        {
            m_val = other.m_val;

            return *this;
        }

        constexpr immutable& operator=(immutable&& other) noexcept
        {
            m_val = std::move(other.m_val);

            other.m_moved = true;

            return *this;
        }

        constexpr bool operator == (const immutable& other) const noexcept
        {
            return m_val == other.m_val;
        }

        constexpr bool operator == (const T& val) const noexcept
        {
            return m_val == val;
        }

        constexpr const T& operator*() const noexcept
        {
            ensureNotMoved();

            return m_val;
        }

        constexpr const T* operator->() const noexcept
        {
            ensureNotMoved();

            return &m_val;
        }

    private:

        constexpr void ensureNotMoved() const
        {
            if (m_moved)
            {
                throw std::runtime_error("An attempt to use an immutable object that has been moved.");
            }
        }
        
        bool m_moved = false;

        T m_val;
    };
}
