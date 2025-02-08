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

        void FromJson(const QJsonValue & jv, value_type & v)
        {
            JsonSerializer<T> formatter;
            T val;
            formatter.FromJson(jv, val);
            v = val;
        }

        void ToJson(const value_type & v, QJsonValue & jv)
        {
            JsonSerializer<T> formatter;
            formatter.ToJson(v.get(), jv);
        }
    };
}
