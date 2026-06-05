#pragma once

#include "BoostExtras/Json/JsonHelpers.h"
#include "BoostExtras/Json/JsonSerializer.h"

#include "Awl/Decimal.h"

#include <format>
#include <sstream>
#include <stdexcept>

namespace awl::boost_json
{
    template <typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate>
    class JsonSerializer<awl::decimal<UInt, exp_len, DataTemplate>>
    {
    public:

        using Decimal = awl::decimal<UInt, exp_len, DataTemplate>;

        void fromJson(const boost::json::value& jv, Decimal& val)
        {
            if (jv.is_string())
            {
                const std::string text(jv.as_string());

                try
                {
                    val = Decimal::from_string(std::string_view(text));
                }
                catch (const std::runtime_error&)
                {
                    throw JsonException(std::format(_T("Can't convert '{}' to decimal."), text));
                }
            }
            else if (jv.is_number())
            {
                const double d_val =
                    jv.is_double() ? jv.as_double() :
                    jv.is_int64() ? static_cast<double>(jv.as_int64()) :
                    static_cast<double>(jv.as_uint64());
                const int64_t int_val = static_cast<int64_t>(d_val);

                if (d_val == int_val)
                {
                    val = Decimal(int_val, 0);
                }
                else
                {
                    val = Decimal::make_decimal(d_val, Decimal::max_exponent());
                    val.normalize();
                }
            }
            else
            {
                throw JsonException(std::format(
                    _T("Can't convert value of type: {} to decimal."),
                    typeToString(jv.kind())));
            }
        }

        void toJson(const Decimal& val, boost::json::value& jv)
        {
            std::ostringstream out;
            out << val;
            jv = out.str();
        }
    };
}
