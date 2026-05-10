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
        
        constexpr unique_ptr() noexcept : _p(nullptr)
        {}

        explicit constexpr unique_ptr(T * p) noexcept : _p(p)
        {}

        unique_ptr(const unique_ptr& other) = delete;

        constexpr unique_ptr(unique_ptr&& other) noexcept : _p(other._p)
        {
            other._p = nullptr;
        }

        constexpr ~unique_ptr()
        {
            destroy();
        }

        constexpr bool operator == (const unique_ptr& other) const noexcept
        {
            return _p == other._p;
        }

        constexpr bool operator != (const unique_ptr& other) const noexcept
        {
            return !operator==(other);
        }

        constexpr bool operator == (const T* p) const noexcept
        {
            return _p == p;
        }

        constexpr bool operator != (const T* p) const noexcept
        {
            return !operator==(p);
        }

        unique_ptr& operator=(const unique_ptr& other) = delete;

        constexpr unique_ptr& operator=(unique_ptr&& other) noexcept
        {
            reset(other._p);

            other._p = nullptr;

            return *this;
        }

        constexpr T& operator*() const noexcept
        {
            return *_p;
        }

        constexpr T* operator->() const noexcept
        {
            return _p;
        }

        constexpr operator bool() const noexcept
        {
            return _p != nullptr;
        }

        constexpr T* get() const noexcept
        {
            return _p;
        }

        constexpr void reset(T* p) noexcept
        {
            destroy();

            _p = p;
        }
        
        constexpr T* release() noexcept
        {
            T* saved_p = _p;

            _p = nullptr;
            
            return saved_p;
        }

    private:
        
        constexpr void destroy() noexcept
        {
            delete _p;
        }

        T* _p;
    };

    template <class T, class... Args>
    unique_ptr<T> make_unique(Args&&... args)
    {
        return unique_ptr(new T(std::forward<Args>(args)...));
    }
}
