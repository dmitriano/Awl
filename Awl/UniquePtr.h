#pragma once

#include <cassert>
#include <utility>

namespace awl
{
    //A unique_ptr that does not prevent the access to the object being destroyed.
    //std::unique_ptr's assignment operator clears its internal pointer first and then deletes
    //the object and thus prevents the access to the object being destroyed.
    template <class T>
    class unique_ptr
    {
    public:
        
        constexpr unique_ptr() : m_p(nullptr)
        {
        }

        constexpr unique_ptr(T * p) : m_p(p)
        {
        }

        unique_ptr(const unique_ptr& other) = delete;

        constexpr unique_ptr(unique_ptr&& other) : m_p(other.m_p)
        {
            other.m_p = nullptr;
        }

        constexpr ~unique_ptr()
        {
            Destroy();
        }

        unique_ptr& operator=(const unique_ptr& other) = delete;

        constexpr unique_ptr& operator=(unique_ptr&& other)
        {
            Destroy();

            m_p = other.m_p;

            other.m_p = nullptr;

            return *this;
        }

        constexpr T& operator*() const
        {
            return *m_p;
        }

        constexpr T* operator->() const
        {
            return m_p;
        }

        constexpr operator bool() const
        {
            return m_p != nullptr;
        }

        constexpr void Destroy()
        {
            delete m_p;
        }

        constexpr T* get() const
        {
            return m_p;
        }

    private:
        
        T* m_p;
    };

    template <class T, class... Args>
    unique_ptr<T> make_unique(Args&&... args)
    {
        return unique_ptr(new T(std::forward<Args>(args)...));
    }
}
