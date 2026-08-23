#pragma once

#include "BoostExtras/Json/JsonSerializer.h"

#include <atomic>

namespace awl
{
    template <class T>
    class JsonSerializer<std::atomic<T>>
    {
    public:

        using value_type = std::atomic<T>;

        void fromJson(const boost::json::value& jv, value_type& atomic_val)
        {
            JsonSerializer<T> formatter;
            T val;
            formatter.fromJson(jv, val);
            atomic_val = val;
        }

        void toJson(const value_type& atomic_val, boost::json::value& jv)
        {
            JsonSerializer<T> formatter;
            formatter.toJson(atomic_val.load(), jv);
        }
    };
}
