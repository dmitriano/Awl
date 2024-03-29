/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>

namespace awl::io
{
    constexpr void StdCopy(const uint8_t * begin, const uint8_t * end, uint8_t * out)
    {
        const uint8_t * p = begin;
        while (p != end)
        {
            *out++ = *p++;
        }
    }

    template <class T>
    constexpr void PlainCopy(uint8_t * p_dest, const uint8_t * p_src)
    {
        T * dest = reinterpret_cast<T *>(p_dest);
        //store to misaligned address:
        const T * src = reinterpret_cast<const T *>(p_src);
        *dest = *src;
    }
}
