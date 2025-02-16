/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"
#include "QtExtras/Json/JsonException.h"
#include "QtExtras/Json/TypeHint.h"

#include "Awl/Reflection.h"

namespace awl
{
    template <class T> requires std::is_class_v<T>&& awl::is_reflectable_v<T>
    class JsonSerializer<T>
    {
    public:

        void FromJson(const QJsonValue & jv, T & obj)
        {
            EnsureType(jv, QJsonValue::Object);
            QJsonObject jo = jv.toObject();

            awl::for_each_index(obj.as_tuple(), [&obj, &jo](auto & field_val, size_t index)
            {
                //Remove reference and const.
                using FieldType = std::decay_t<decltype(field_val)>;

                JsonSerializer<FieldType> formatter;

                const std::string& cpp_key = obj.get_member_names()[index];

                QLatin1String key(cpp_key.c_str());

                const QJsonValue& key_jv = jo[key];

                try
                {
                    formatter.FromJson(key_jv, field_val);
                }
                catch (JsonException& e)
                {
                    e.append({ key_jv.type(), type_hint<FieldType>(), cpp_key});

                    throw e;
                }
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
