/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"

#include <functional>

namespace awl
{
    template <class T>
    class JsonSerializer<std::reference_wrapper<T>>
    {
    public:

        using value_type = std::reference_wrapper<T>;

        void fromJson(const QJsonValue & jv, value_type & v)
        {
            JsonSerializer<T> formatter;
            T val;
            formatter.fromJson(jv, val);
            v = val;
        }

        void toJson(const value_type & v, QJsonValue & jv)
        {
            JsonSerializer<T> formatter;
            formatter.toJson(v.get(), jv);
        }
    };
}
