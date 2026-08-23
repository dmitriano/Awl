#pragma once

#include "BoostExtras/Json/JsonHelpers.h"
#include "BoostExtras/Json/JsonSerializer.h"

#include "Awl/TupleHelpers.h"

#include <tuple>
#include <type_traits>

namespace awl
{
    template <typename... Args>
    class JsonSerializer<std::tuple<Args...>>
    {
    public:

        using value_type = std::tuple<Args...>;

        void fromJson(const boost::json::value& jv, value_type& val)
        {
            const boost::json::array& ja = asArray(jv);

            awl::for_each_index(val, [&ja](auto& field_val, size_t index)
            {
                if (index >= ja.size())
                {
                    throw JsonException(_T("Tuple JSON array is too short."));
                }

                JsonSerializer<std::decay_t<decltype(field_val)>> formatter;
                formatter.fromJson(ja[index], field_val);
            });
        }

        void toJson(const value_type& val, boost::json::value& jv)
        {
            boost::json::array ja;

            awl::for_each(val, [&ja](auto& field_val)
            {
                JsonSerializer<std::decay_t<decltype(field_val)>> formatter;
                boost::json::value field_jv;
                formatter.toJson(field_val, field_jv);
                ja.push_back(std::move(field_jv));
            });

            jv = std::move(ja);
        }
    };
}
