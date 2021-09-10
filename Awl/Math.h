/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <type_traits>

//#define M_PI_4     0.785398163397448309616  // pi/4
//#define M_1_PI     0.318309886183790671538  // 1/pi
//#define M_2_PI     0.636619772367581343076  // 2/pi
//#define M_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
//#define M_SQRT2    1.41421356237309504880   // sqrt(2)
//#define M_SQRT1_2  0.707106781186547524401  // 1/sqrt(2)
//#endif

namespace awl
{
    namespace math
    {
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

        //GCC 4.9 does not have std::round and std::lround
        template <class I, class F>
        inline typename std::enable_if<std::is_integral<I>::value && std::is_floating_point<F>::value, I>::type round(F f)
        {
#ifdef __GNUC__
            return static_cast<I>(f + 0.5f);
#else
            return static_cast<I>(std::lruond(f));
#endif
        }
    }
}
