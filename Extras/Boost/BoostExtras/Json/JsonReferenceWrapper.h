#pragma once

#include "BoostExtras/Json/JsonSerializer.h"

#include <functional>

namespace awl::boost_json
{
    template <class T>
    class JsonSerializer<std::reference_wrapper<T>>
    {
    public:

        using value_type = std::reference_wrapper<T>;

        void fromJson(const boost::json::value& jv, value_type& v)
        {
            JsonSerializer<T> formatter;
            formatter.fromJson(jv, v.get());
        }

        void toJson(const value_type& v, boost::json::value& jv)
        {
            JsonSerializer<T> formatter;
            formatter.toJson(v.get(), jv);
        }
    };
}
