#pragma once

#include <stdint.h>
#include <utility>

namespace awl
{
    //! An object that is destroyed explicitly by calling its destructor with p->~T(). Can be used with destructable singleton pattern.
    template <class T>
    class Destructible
    {
    public:

        explicit Destructible()
        {
            new(m_storage) T();
        }

        template <class ...Args>
        explicit Destructible(Args... args)
        {
            new(m_storage) T(std::forward<Args...>(args...));
        }
        
        explicit Destructible(const T & t)
        {
            new(m_storage) T(t);
        }

        explicit Destructible(T && t)
        {
            new(m_storage) T(std::forward<T>(t));
        }

        void Destroy()
        {
            get()->T::~T();
        }

        operator T * ()
        {
            return get();
        }

        ~Destructible()
        {
        }
    
    private:
        
        T * get()
        {
            return reinterpret_cast<T *>(m_storage);
        }

        uint8_t m_storage[sizeof(T)];
    };
}
