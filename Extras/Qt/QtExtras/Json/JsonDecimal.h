#pragma once

#include "QtExtras/Json/JsonSerializer.h"
#include "Awl/StringFormat.h"
#include "Awl/Decimal.h"

namespace awl
{
    template <typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate>
    class JsonSerializer<awl::decimal<UInt, exp_len, DataTemplate>>
    {
    public:

        using Decimal = Decimal<UInt, exp_len, DataTemplate>;

        void FromJson(const QJsonValue& jv, Decimal& val)
        {
            switch (jv.type())
            {
                case QJsonValue::String:
                {
                    const std::string text = jv.toString().toStdString();

                    try
                    {
                        val = Decimal::from_string(std::string_view(text));
                    }
                    catch (const std::runtime_error&)
                    {
                        throw JsonException(awl::format() << "Can't convert '" << text << "' to decimal.");
                    }

                    break;
                }
                case QJsonValue::Double:
                {
                    const double d_val = jv.toDouble();

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
                    
                    break;
                }
                default:

                    throw JsonException(awl::format() << _T("Can't convert value of type: ") << TypeToString(jv.type()) << _T(" to decimal."));

                    break;
            }
        }

        void ToJson(const Decimal& val, QJsonValue& jv)
        {
            std::ostringstream out;

            out << val;
            
            jv = QString::fromStdString(out.str());
        }
    };
}
