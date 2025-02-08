#pragma once

#include "Qtil/Json/JsonSerializer.h"

#include "Awl/Reflection.h"

namespace qtil
{
    template <class T>
    class JsonSerializer<T, std::enable_if_t<std::is_class_v<T> && awl::is_reflectable_v<T>>>
    {
    public:

        void FromJson(const QJsonValue & jv, T & obj)
        {
            EnsureType(jv, QJsonValue::Object);
            QJsonObject jo = jv.toObject();

            awl::for_each_index(obj.as_tuple(), [&obj, &jo](auto & field_val, size_t index)
            {
                //Remove reference and const.
                JsonSerializer<std::decay_t<decltype(field_val)>> formatter;

                QLatin1String key(obj.get_member_names()[index].c_str());
                formatter.FromJson(jo[key], field_val);
            });
        }

        void ToJson(const T & obj, QJsonValue & jv)
        {
            QJsonObject jo;

            awl::for_each_index(obj.as_const_tuple(), [&obj, &jo](auto & field_val, size_t index)
            {
                //Remove reference and const.
                JsonSerializer<std::decay_t<decltype(field_val)>> formatter;

                QLatin1String key(obj.get_member_names()[index].c_str());
                QJsonValue field_jv;
                formatter.ToJson(field_val, field_jv);
                jo[key] = field_jv;
            });

            jv = jo;
        }
    };
}
