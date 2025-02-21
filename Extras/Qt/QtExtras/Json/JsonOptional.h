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

        void FromJson(const QJsonValue & jv, value_type & opt)
        {
            if (!IsNull(jv))
            {
                JsonSerializer<T> formatter;
                T val;
                formatter.FromJson(jv, val);
                opt = val;
            }
            else
            {
                opt = {};
            }
        }

        void ToJson(const value_type & v, QJsonValue & jv)
        {
            if (v.has_value())
            {
                JsonSerializer<T> formatter;
                formatter.ToJson(v.value(), jv);
            }
            else
            {
                jv = QJsonValue(QJsonValue::Null);
            }
        }
    };
}
