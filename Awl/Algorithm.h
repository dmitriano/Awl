#pragma once

#include <algorithm>

namespace awl
{
    //std::clamp in C++17 is a bit different, it is constexpr and returns const T&

    template<class T, class Compare>
    void clamp(T& v, const T& lo, const T& hi, Compare less)
    {
        if (less(v, lo))
        {
            v = lo;
        }
        else
        {
            if (less(hi, v))
            {
                v = hi;
            }
        }
    }

    template<class T>
    void clamp(T& v, const T& lo, const T& hi)
    {
        clamp(v, lo, hi, std::less<>());
    }
}
