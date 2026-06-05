#pragma once

#include "BoostExtras/Json/JsonHelpers.h"
#include "BoostExtras/Json/JsonSerializer.h"

#include <charconv>
#include <concepts>
#include <cstdlib>
#include <format>
#include <limits>
#include <string>
#include <type_traits>

namespace awl::boost_json
{
    template <>
    class JsonSerializer<bool>
    {
    public:

        void fromJson(const boost::json::value& jv, bool& val)
        {
            ensureType(jv, boost::json::kind::bool_);
            val = jv.as_bool();
        }

        void toJson(bool val, boost::json::value& jv)
        {
            jv = val;
        }
    };

    template <std::integral T> requires (!std::is_same_v<T, bool>)
    class JsonSerializer<T>
    {
    public:

        void fromJson(const boost::json::value& jv, T& val)
        {
            if (jv.is_int64())
            {
                val = checkedCast(jv.as_int64());
            }
            else if (jv.is_uint64())
            {
                val = checkedCast(jv.as_uint64());
            }
            else if (jv.is_double())
            {
                const double d_val = jv.as_double();
                const int64_t i_val = static_cast<int64_t>(d_val);

                if (static_cast<double>(i_val) != d_val)
                {
                    throw JsonException(_T("JSON double value cannot be converted to an integral value without precision loss."));
                }

                val = checkedCast(i_val);
            }
            else if (jv.is_string())
            {
                const std::string text(jv.as_string());
                T parsed{};
                const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), parsed);

                if (ec != std::errc{} || ptr != text.data() + text.size())
                {
                    throw JsonException(std::format(_T("Can't convert '{}' to an integral value."), text));
                }

                val = parsed;
            }
            else
            {
                throw JsonException(std::format(
                    _T("Expected numeric or string value, actual value type: {}"),
                    typeToString(jv.kind())));
            }
        }

        void toJson(T val, boost::json::value& jv)
        {
            if constexpr (std::is_signed_v<T>)
            {
                jv = static_cast<int64_t>(val);
            }
            else
            {
                jv = static_cast<uint64_t>(val);
            }
        }

    private:

        template <class U>
        static T checkedCast(U val)
        {
            if constexpr (std::is_signed_v<U> == std::is_signed_v<T>)
            {
                if (val < static_cast<U>(std::numeric_limits<T>::min()) ||
                    val > static_cast<U>(std::numeric_limits<T>::max()))
                {
                    throw JsonException(_T("JSON integer value is out of target type range."));
                }
            }
            else if constexpr (std::is_signed_v<U>)
            {
                if (val < 0 || static_cast<std::make_unsigned_t<U>>(val) > std::numeric_limits<T>::max())
                {
                    throw JsonException(_T("JSON integer value is out of target type range."));
                }
            }
            else
            {
                if (val > static_cast<std::make_unsigned_t<T>>(std::numeric_limits<T>::max()))
                {
                    throw JsonException(_T("JSON integer value is out of target type range."));
                }
            }

            return static_cast<T>(val);
        }
    };

    template <std::floating_point T>
    class JsonSerializer<T>
    {
    public:

        void fromJson(const boost::json::value& jv, T& val)
        {
            if (jv.is_double())
            {
                val = static_cast<T>(jv.as_double());
            }
            else if (jv.is_int64())
            {
                val = static_cast<T>(jv.as_int64());
            }
            else if (jv.is_uint64())
            {
                val = static_cast<T>(jv.as_uint64());
            }
            else if (jv.is_string())
            {
                const std::string text(jv.as_string());
                char* end = nullptr;
                const double parsed = std::strtod(text.c_str(), &end);

                if (end != text.c_str() + text.size())
                {
                    throw JsonException(std::format(_T("Can't convert '{}' to a floating point value."), text));
                }

                val = static_cast<T>(parsed);
            }
            else
            {
                throw JsonException(std::format(
                    _T("Expected numeric or string value, actual value type: {}"),
                    typeToString(jv.kind())));
            }
        }

        void toJson(T val, boost::json::value& jv)
        {
            jv = static_cast<double>(val);
        }
    };
}
