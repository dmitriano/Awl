/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>

namespace awl
{
    template <class T>
    constexpr std::byte* byte_cast(T* data)
    {
        return reinterpret_cast<std::byte*>(data);
    }

    template <class T>
    constexpr const std::byte* byte_cast(const T* data)
    {
        return reinterpret_cast<const std::byte*>(data);
    }

    template <class T>
    constexpr char* char_cast(T* data)
    {
        return reinterpret_cast<char*>(data);
    }

    template <class T>
    constexpr const char* char_cast(const T* data)
    {
        return reinterpret_cast<const char*>(data);
    }
}
