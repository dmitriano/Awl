#pragma once

#include "BoostExtras/Json/JsonSerializer.h"

#include "Awl/EnumTraits.h"
#include "Awl/StringFormat.h"

#include <format>
#include <type_traits>

namespace awl
{
    template <class T> requires std::is_enum_v<T>
    class JsonSerializer<T>
    {
    public:

        using value_type = T;

        void fromJson(const boost::json::value& jv, value_type& val)
        {
            JsonSerializer<std::string> formatter;
            std::string str_val;
            formatter.fromJson(jv, str_val);

            try
            {
                // Accept enum names case-insensitively, like AWL command-line attributes.
                val = awl::enum_from_string_i<T>(str_val);
            }
            catch (const std::runtime_error&)
            {
                throw JsonException(std::format(_T("Wrong enum value '{}'."), str_val));
            }
        }

        void toJson(const value_type& val, boost::json::value& jv)
        {
            JsonSerializer<std::string> formatter;
            formatter.toJson(awl::enum_to_string(val), jv);
        }
    };
}
