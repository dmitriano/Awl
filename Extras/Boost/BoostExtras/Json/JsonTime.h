#pragma once

#include "BoostExtras/Json/JsonException.h"
#include "BoostExtras/Json/JsonHelpers.h"
#include "BoostExtras/Json/JsonSerializer.h"

#include "Awl/Duration.h"

#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace awl
{
    template <class Clock, class Duration>
    class JsonSerializer<std::chrono::time_point<Clock, Duration>>
    {
    public:

        using value_type = std::chrono::time_point<Clock, Duration>;

        void fromJson(const boost::json::value& jv, value_type& v)
        {
            using Milliseconds = std::chrono::milliseconds;
            std::int64_t ms{};
            if (jv.is_int64())
            {
                ms = jv.as_int64();
            }
            else if (jv.is_uint64() && jv.as_uint64() <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
            {
                ms = static_cast<std::int64_t>(jv.as_uint64());
            }
            else if (jv.is_double() && std::isfinite(jv.as_double()) && std::trunc(jv.as_double()) == jv.as_double()
                && jv.as_double() >= -std::ldexp(1.0, 63) && jv.as_double() < std::ldexp(1.0, 63))
            {
                ms = static_cast<std::int64_t>(jv.as_double());
            }
            else if (jv.is_string())
            {
                const std::string_view text = asString(jv);
                const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), ms);
                if (error != std::errc{} || end != text.data() + text.size())
                {
                    throw JsonException("Invalid epoch-millisecond integer.");
                }
            }
            else
            {
                throw JsonException("Expected epoch milliseconds as a signed 64-bit integer.");
            }

            try
            {
                v = value_type(time_detail::checked_duration_cast<Duration>(Milliseconds(ms)));
            }
            catch (const GeneralException& error)
            {
                throw JsonException(error.message());
            }
        }

        void toJson(const value_type& v, boost::json::value& jv)
        {
            using Milliseconds = std::chrono::milliseconds;
            using Factor = std::ratio_divide<typename Duration::period, std::milli>;
            try
            {
                if constexpr (std::is_integral_v<typename Duration::rep> && Factor::num == 1)
                {
                    // Preserve the existing truncation of sub-millisecond ticks,
                    // without converting 64-bit integer epochs through double.
                    const std::chrono::duration<typename Duration::rep, std::milli> truncated(
                        v.time_since_epoch().count() / Factor::den);
                    jv = time_detail::checked_duration_cast<Milliseconds>(truncated).count();
                }
                else if constexpr (std::is_integral_v<typename Duration::rep> && Factor::den == 1)
                {
                    jv = time_detail::checked_duration_cast<Milliseconds>(v.time_since_epoch()).count();
                }
                else
                {
                    const long double ms = std::chrono::duration<long double, std::milli>(v.time_since_epoch()).count();
                    jv = time_detail::checked_duration_cast<Milliseconds>(
                        std::chrono::duration<long double, std::milli>(std::trunc(ms))).count();
                }
            }
            catch (const GeneralException& error)
            {
                throw JsonException(error.message());
            }
        }
    };

    template <class Rep, class Period>
    class JsonSerializer<std::chrono::duration<Rep, Period>>
    {
    public:

        using value_type = std::chrono::duration<Rep, Period>;

        void fromJson(const boost::json::value& jv, value_type& v)
        {
            const std::string_view text = asString(jv);
            try
            {
                v = parse_duration<value_type>(text);
            }
            catch (const GeneralException& error)
            {
                throw JsonException(error.message());
            }
        }

        void toJson(const value_type& v, boost::json::value& jv)
        {
            try
            {
                jv = duration_to_string<char>(v);
            }
            catch (const GeneralException& error)
            {
                throw JsonException(error.message());
            }
        }
    };
}
