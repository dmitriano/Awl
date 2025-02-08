#pragma once

#include "Qtil/Json/JsonSerializer.h"

#include "Awl/TupleHelpers.h"

namespace qtil
{
    template <typename... Args>
    class JsonSerializer<std::tuple<Args...>>
    {
    public:

        using value_type = std::tuple<Args...>;

        void FromJson(const QJsonValue & jv, value_type& val)
        {
            EnsureType(jv, QJsonValue::Array);
            QJsonArray ja = jv.toArray();

            awl::for_each_index(val, [&ja](auto & field_val, size_t index)
            {
                //Remove reference and const.
                JsonSerializer<std::decay_t<decltype(field_val)>> formatter;

                QJsonValue jv = ja[index];

                formatter.FromJson(jv, field_val);
            });
        }

        void ToJson(const value_type& val, QJsonValue & jv)
        {
            QJsonArray ja;

            awl::for_each(val, [&ja](auto & field_val)
            {
                //Remove reference and const.
                JsonSerializer<std::decay_t<decltype(field_val)>> formatter;

                QJsonValue field_jv;
                formatter.ToJson(field_val, field_jv);
                ja.append(field_jv);
            });

            jv = ja;
        }
    };
}
