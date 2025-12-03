/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/DataCast.h"

#include <stdint.h>
#include <utility>
#include <type_traits>

namespace awl
{
    //! An object that is destroyed explicitly by calling its destructor with p->~T().
    template <class T>
    class Destructible
    {
    public:

        explicit Destructible()
        {
            new(address()) T();
        }

        template <class ...Args>
        explicit Destructible(Args&&... args)
        {
            new(address()) T(std::forward<Args...>(args) ...);
        }
        
        explicit Destructible(const T & t)
        {
            new(address()) T(t);
        }

        explicit Destructible(T && t)
        {
            new(address()) T(std::forward<T>(t));
        }

        void Destroy()
        {
            get()->T::~T();
        }

        operator T * ()
        {
            return get();
        }

        T * operator -> ()
        {
            return get();
        }

        ~Destructible()
        {
        }
    
    private:
        
        T * get()
        {
            return launder_cast<T>(&m_storage);
        }

        void * address()
        {
            return address_cast(m_storage);
        }

        //Properly aligned uninitialized storage for T.
        alignas(alignof(T)) uint8_t m_storage[sizeof(T)];
    };
}
