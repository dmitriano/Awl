/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdexcept>
#include <cstddef>

namespace awl
{
    template <class T>
    class immutable
    {
    public:

        constexpr immutable(T val) noexcept : m_val(std::move(val)) {}
        
        constexpr immutable(const immutable& other) : m_val(other.m_val) {}

        constexpr immutable(immutable&& other) noexcept : m_val(other.m_val)
        {
            other.markAsMoved();
        }

        constexpr immutable& operator=(const immutable& other)
        {
            m_val = other.m_val;

            return *this;
        }

        constexpr immutable& operator=(immutable&& other) noexcept
        {
            m_val = std::move(other.m_val);

            other.markAsMoved();

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

        constexpr const T* operator->() const noexcept
        {
            ensureNotMoved();

            return &m_val;
        }

    private:


// This can be done via a template paramter like template<class T, Checker = FakeChecker<T>>
#ifdef AWL_DEBUG_IMMUTABLE
        
        constexpr void markAsMoved()
        {
            m_moved = true;
        }

        constexpr void ensureNotMoved() const
        {
            if (m_moved)
            {
                // This will result in std::terminate, because operator->() is noexcept.
                throw std::runtime_error("An attempt to use an immutable object that was moved.");
            }
        }

        bool m_moved = false;

#else
        
        constexpr void markAsMoved() {}

        constexpr void ensureNotMoved() const {}

#endif

        T m_val;
    };

    template <class T, class... Args>
    constexpr immutable<T> make_immutable(Args&&... args)
    {
        return T{ std::forward<Args>(args)... };
    }
}
