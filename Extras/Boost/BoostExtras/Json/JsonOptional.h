#pragma once

#include "BoostExtras/Json/JsonHelpers.h"
#include "BoostExtras/Json/JsonSerializer.h"

#include <optional>

namespace awl::boost_json
{
    template <class T>
    class JsonSerializer<std::optional<T>>
    {
    public:

        using value_type = std::optional<T>;

        void fromJson(const boost::json::value& jv, value_type& opt)
        {
            if (isNull(jv))
            {
                opt = {};
                return;
            }

            JsonSerializer<T> formatter;
            T val;
            formatter.fromJson(jv, val);
            opt = std::move(val);
        }

        void toJson(const value_type& opt, boost::json::value& jv)
        {
            if (opt)
            {
                JsonSerializer<T> formatter;
                formatter.toJson(*opt, jv);
            }
            else
            {
                jv = nullptr;
            }
        }
    };
}
