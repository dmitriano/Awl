#pragma once

//#define M_PI_4     0.785398163397448309616  // pi/4
//#define M_1_PI     0.318309886183790671538  // 1/pi
//#define M_2_PI     0.636619772367581343076  // 2/pi
//#define M_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
//#define M_SQRT2    1.41421356237309504880   // sqrt(2)
//#define M_SQRT1_2  0.707106781186547524401  // 1/sqrt(2)
//#endif

namespace awl { namespace math {

	template <class T> constexpr T pi() { return static_cast<T>(3.14159265358979323846); }

	template <class T> constexpr T half_pi() { return static_cast<T>(1.57079632679489661923); }

	template <class T> constexpr T quarter_pi() { return static_cast<T>(0.785398163397448309616); }

	template <class T>
	inline T degrees_to_radians(T rad)
	{
		return rad / static_cast<T>(180.0L) * pi<T>();
	}

	template <class T>
	inline T raidans_to_degrees(T deg)
	{
		return deg / pi<T>() * static_cast<T>(180.0L);
	}
}}
