/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <type_traits>
#include <functional>

namespace awl::math
{
    //Constants can be replaced with std::numbers in C++20.

    template <class T> constexpr typename std::enable_if<std::is_floating_point<T>::value, T>::type pi() { return static_cast<T>(3.14159265358979323846); }

    template <class T> constexpr typename std::enable_if<std::is_floating_point<T>::value, T>::type half_pi() { return static_cast<T>(1.57079632679489661923); }

    template <class T> constexpr typename std::enable_if<std::is_floating_point<T>::value, T>::type quarter_pi() { return static_cast<T>(0.785398163397448309616); }

    template <class T>
    constexpr typename std::enable_if<std::is_floating_point<T>::value, T>::type degrees_to_radians(T rad)
    {
        return rad / static_cast<T>(180.0L) * pi<T>();
    }

    template <class T>
    constexpr typename std::enable_if<std::is_floating_point<T>::value, T>::type raidans_to_degrees(T deg)
    {
        return deg / pi<T>() * static_cast<T>(180.0L);
    }

    template <class I, class F>
    inline typename std::enable_if<std::is_integral<I>::value && std::is_floating_point<F>::value, I>::type round(F f)
    {
        return static_cast<I>(f + 0.5f);
    }

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
