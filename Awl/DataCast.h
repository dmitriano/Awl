/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <new>

namespace awl
{
    template <class To, class From>
    constexpr To* launder_cast(From* data)
    {
        return std::launder(reinterpret_cast<To*>(data));
    }

    template <class T>
    void* address_cast(T& storage)
    {
        return reinterpret_cast<void*>(&storage);
    }

    template <class T>
    constexpr uint8_t* mutable_data_cast(T* data)
    {
        return launder_cast<uint8_t>(data);
    }

    template <class T>
    constexpr const uint8_t* const_data_cast(const T* data)
    {
        return launder_cast<const uint8_t>(data);
    }
}
