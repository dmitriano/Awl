#pragma once

#include <cassert>
#include <utility>

namespace awl
{
    // At least in MSVC:
    // std::unique_ptr::reset() function clears its internal pointer first and then deletes the object and thus
    // prevents access to the object being destroyed. If the deleter throws an exception std::unique_ptr keeps new object and looses old.
    // IMHO this behaviour is not intuitive. I would expect std::unique_ptr to keep old object if reset did not succeed,
    // so in catch block I have the access to both old file and new file. For example, if std::fflush failed because
    // there is no space left on the disk, in catch block I free some space and call reset again.
    
    //The implementation is not complete yet, it does not have deleter and does not cast from a derived type.
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

        constexpr bool operator == (const unique_ptr& other) const
        {
            return m_p == other.m_p;
        }

        constexpr bool operator != (const unique_ptr& other) const
        {
            return !operator==(other);
        }

        constexpr bool operator == (const T* p) const
        {
            return m_p == p;
        }

        constexpr bool operator != (const T* p) const
        {
            return !operator==(p);
        }

        unique_ptr& operator=(const unique_ptr& other) = delete;

        constexpr unique_ptr& operator=(unique_ptr&& other)
        {
            reset(other.m_p);

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
            Destroy();

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
