/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"

#include <atomic>

namespace awl
{
    template <class T>
    class JsonSerializer<std::atomic<T>>
    {
    public:

        using value_type = std::atomic<T>;

        void FromJson(const QJsonValue& jv, value_type& atomic_val)
        {
            JsonSerializer<T> formatter;
            T val;
            formatter.FromJson(jv, val);
            atomic_val = val;
        }

        void ToJson(const value_type & atomic_val, QJsonValue & jv)
        {
            JsonSerializer<T> formatter;
            formatter.ToJson(atomic_val.load(), jv);
        }
    };
}
