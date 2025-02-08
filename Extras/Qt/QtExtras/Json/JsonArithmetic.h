#pragma once

#include "Qtil/Json/JsonSerializer.h"
#include "Qtil/Format.h"
#include "Qtil/DecimalType.h"

namespace qtil
{
    template <>
    class JsonSerializer<bool>
    {
    public:

        void FromJson(const QJsonValue& jv, bool& val)
        {
            EnsureType(jv, QJsonValue::Bool);
            val = jv.toBool();
        }

        void ToJson(bool val, QJsonValue& jv)
        {
            jv = val;
        }
    };

    template <class T>
    class JsonSerializer<T, std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>>
    {
    public:

        void FromJson(const QJsonValue& jv, T& val)
        {
            switch (jv.type())
            {
            case QJsonValue::Double:
                val = jv.toDouble();
                break;

            case QJsonValue::String:
            {
                bool ok;
                const QString strVal = jv.toString();
                val = strVal.toDouble(&ok);
                if (!ok)
                {
                    throw JsonException(awl::format() << _T("Can't convert a JSON value from String to Double."));
                }
                break;
            }

            default:
                throw JsonException(awl::format() << _T("Expected value of Double or String type, actul value type: ") << TypeToString(jv.type()));
                break;
            }
        }

        void ToJson(T val, QJsonValue& jv)
        {
            //It can't assign directly.
            //jv.fromVariant(QVariant::fromValue(val));

            //QJsonValue does not construct from uint64_t
            if constexpr (std::is_integral_v<T>)
            {
                jv = QJsonValue(static_cast<qint64>(val));
            }
            else
            {
                jv = QJsonValue(static_cast<double>(val));
            }
        }
    };

    template <>
    class JsonSerializer<qtil::decimal>
    {
    public:

        void FromJson(const QJsonValue& jv, qtil::decimal& val)
        {
            switch (jv.type())
            {
                case QJsonValue::String:
                {
                    const std::string text = jv.toString().toStdString();

                    try
                    {
                        val = qtil::decimal::from_string(std::string_view(text));
                    }
                    catch (const std::runtime_error&)
                    {
                        throw JsonException(qtil::Format() << "Can't convert '" << text << "' to decimal.");
                    }

                    break;
                }
                case QJsonValue::Double:
                {
                    const double d_val = jv.toDouble();

                    const int64_t int_val = static_cast<int64_t>(d_val);

                    if (d_val == int_val)
                    {
                        val = qtil::decimal(int_val, 0);
                    }
                    else
                    {
                        val = qtil::decimal::make_decimal(d_val, qtil::decimal::max_exponent());

                        val.normalize();
                    }
                    
                    break;
                }
                default:

                    throw JsonException(awl::format() << _T("Can't convert value of type: ") << TypeToString(jv.type()) << _T(" to decimal."));

                    break;
            }
        }

        void ToJson(const qtil::decimal& val, QJsonValue& jv)
        {
            std::ostringstream out;

            out << val;
            
            jv = QString::fromStdString(out.str());
        }
    };
}
