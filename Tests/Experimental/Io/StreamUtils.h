/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace awl::io
{
    constexpr void StdCopy(const std::byte * begin, const std::byte * end, std::byte * out)
    {
        const std::byte * p = begin;
        while (p != end)
        {
            *out++ = *p++;
        }
    }

    template <class T>
    constexpr void PlainCopy(std::byte * p_dest, const std::byte * p_src)
    {
        std::memcpy(p_dest, p_src, sizeof(T));
    }
}
