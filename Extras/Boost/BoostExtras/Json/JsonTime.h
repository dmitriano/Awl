#pragma once

#include "BoostExtras/Json/JsonException.h"
#include "BoostExtras/Json/JsonHelpers.h"
#include "BoostExtras/Json/JsonSerializer.h"

#include "Awl/Duration.h"

#include <chrono>
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
            using namespace std::chrono;

            double ms{};

            if (jv.is_string())
            {
                ms = std::stod(std::string(jv.as_string()));
            }
            else if (jv.is_double())
            {
                ms = jv.as_double();
            }
            else if (jv.is_int64())
            {
                ms = static_cast<double>(jv.as_int64());
            }
            else if (jv.is_uint64())
            {
                ms = static_cast<double>(jv.as_uint64());
            }
            else
            {
                throw JsonException(_T("Expected time point as JSON number or string."));
            }

            v = value_type(milliseconds(static_cast<milliseconds::rep>(ms)));
        }

        void toJson(const value_type& v, boost::json::value& jv)
        {
            using namespace std::chrono;
            jv = static_cast<int64_t>(duration_cast<milliseconds>(v.time_since_epoch()).count());
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
