/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"

#include <optional>

namespace awl
{
    template <class T>
    class JsonSerializer<std::optional<T>>
    {
    public:

        using value_type = std::optional<T>;

        void fromJson(const QJsonValue & jv, value_type & opt)
        {
            if (!isNull(jv))
            {
                JsonSerializer<T> formatter;
                T val;
                formatter.fromJson(jv, val);
                opt = val;
            }
            else
            {
                opt = {};
            }
        }

        void toJson(const value_type & v, QJsonValue & jv)
        {
            if (v.has_value())
            {
                JsonSerializer<T> formatter;
                formatter.toJson(v.value(), jv);
            }
            else
            {
                jv = QJsonValue(QJsonValue::Null);
            }
        }
    };
}
