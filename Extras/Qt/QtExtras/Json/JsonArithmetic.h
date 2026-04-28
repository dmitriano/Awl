/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"
#include "QtExtras/Json/JsonHelpers.h"
#include "QtExtras/Json/JsonException.h"
#include "Awl/StringFormat.h"

namespace awl
{
    template <>
    class JsonSerializer<bool>
    {
    public:

        void fromJson(const QJsonValue& jv, bool& val)
        {
            ensureType(jv, QJsonValue::Bool);
            val = jv.toBool();
        }

        void toJson(bool val, QJsonValue& jv)
        {
            jv = val;
        }
    };

    template <class T> requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
    class JsonSerializer<T>
    {
    public:

        void fromJson(const QJsonValue& jv, T& val)
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
                    throw JsonException(_T("Can't convert a JSON value from String to Double."));
                }
                break;
            }

            default:
                throw JsonException(std::format(_T("Expected value of Double or String type, actul value type: {}"), typeToString(jv.type())));
                break;
            }
        }

        void toJson(T val, QJsonValue& jv)
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
}
