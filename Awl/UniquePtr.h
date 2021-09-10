/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cassert>
#include <utility>

namespace awl
{
    // At least in MSVC (and probably in GCC also):
    // std::unique_ptr::reset() function clears its internal pointer first and then deletes the object and thus
    // My idea was that it prevents a recursion when reset throws an exception (from the deleter) and then std::unique_ptr destructor is called,
    // but deleter can't throw exceptions because reset is declared with noexcept.

    //The implementation is not complete yet, it does not have deleter and does not cast from a derived type.
    template <class T>
    class unique_ptr
    {
    public:
        
        constexpr unique_ptr() noexcept : m_p(nullptr)
        {
        }

        explicit constexpr unique_ptr(T * p) noexcept : m_p(p)
        {
        }

        unique_ptr(const unique_ptr& other) = delete;

        constexpr unique_ptr(unique_ptr&& other) noexcept : m_p(other.m_p)
        {
            other.m_p = nullptr;
        }

        constexpr ~unique_ptr()
        {
            Destroy();
        }

        constexpr bool operator == (const unique_ptr& other) const noexcept
        {
            return m_p == other.m_p;
        }

        constexpr bool operator != (const unique_ptr& other) const noexcept
        {
            return !operator==(other);
        }

        constexpr bool operator == (const T* p) const noexcept
        {
            return m_p == p;
        }

        constexpr bool operator != (const T* p) const noexcept
        {
            return !operator==(p);
        }

        unique_ptr& operator=(const unique_ptr& other) = delete;

        constexpr unique_ptr& operator=(unique_ptr&& other) noexcept
        {
            reset(other.m_p);

            other.m_p = nullptr;

            return *this;
        }

        constexpr T& operator*() const noexcept
        {
            return *m_p;
        }

        constexpr T* operator->() const noexcept
        {
            return m_p;
        }

        constexpr operator bool() const noexcept
        {
            return m_p != nullptr;
        }

        constexpr T* get() const noexcept
        {
            return m_p;
        }

        constexpr void reset(T* p) noexcept
        {
            Destroy();

            m_p = p;
        }
        
        constexpr T* release() noexcept
        {
            T* saved_p = m_p;

            m_p = nullptr;
            
            return saved_p;
        }

    private:
        
        constexpr void Destroy() noexcept
        {
            delete m_p;
        }

        T* m_p;
    };

    template <class T, class... Args>
    unique_ptr<T> make_unique(Args&&... args)
    {
        return unique_ptr(new T(std::forward<Args>(args)...));
    }
}
