#pragma once

#include <cassert>
#include <utility>

namespace awl
{
    //A unique_ptr that does not prevent the access to the object being destroyed.
    //std::unique_ptr's assignment operator clears its internal pointer first and then deletes
    //the object and thus prevents the access to the object being destroyed.
    //The implementation is not complete, it does not cast from a derived type.
    template <class T>
    class unique_ptr
    {
    public:
        
        constexpr unique_ptr() : m_p(nullptr)
        {
        }

        explicit constexpr unique_ptr(T * p) : m_p(p)
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

        bool operator == (const unique_ptr& other) const
        {
            return m_p == other.m_p;
        }

        bool operator != (const unique_ptr& other) const
        {
            return !operator==(other);
        }

        bool operator == (const T* p) const
        {
            return m_p == p;
        }

        bool operator != (const T* p) const
        {
            return !operator==(p);
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

        constexpr void reset(T* p)
        {
            m_p = p;
        }
        
        constexpr T* release()
        {
            T* saved_p = m_p;

            m_p = nullptr;
            
            return saved_p;
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
