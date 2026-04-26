/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"

#include "Awl/TupleHelpers.h"

namespace awl
{
    template <typename... Args>
    class JsonSerializer<std::tuple<Args...>>
    {
    public:

        using value_type = std::tuple<Args...>;

        void fromJson(const QJsonValue & jv, value_type& val)
        {
            EnsureType(jv, QJsonValue::Array);
            QJsonArray ja = jv.toArray();

            awl::for_each_index(val, [&ja](auto & field_val, size_t index)
            {
                //Remove reference and const.
                JsonSerializer<std::decay_t<decltype(field_val)>> formatter;

                QJsonValue jv = ja[index];

                formatter.fromJson(jv, field_val);
            });
        }

        void toJson(const value_type& val, QJsonValue & jv)
        {
            QJsonArray ja;

            awl::for_each(val, [&ja](auto & field_val)
            {
                //Remove reference and const.
                JsonSerializer<std::decay_t<decltype(field_val)>> formatter;

                QJsonValue field_jv;
                formatter.toJson(field_val, field_jv);
                ja.append(field_jv);
            });

            jv = ja;
        }
    };
}
