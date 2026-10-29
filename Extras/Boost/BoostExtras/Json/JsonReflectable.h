#pragma once

#include "BoostExtras/Json/JsonHelpers.h"
#include "BoostExtras/Json/JsonSerializer.h"
#include "BoostExtras/Json/TypeHint.h"

#include "Awl/Reflection.h"

#include <format>
#include <string>
#include <type_traits>

namespace awl
{
    template <class T> requires std::is_class_v<T> && awl::reflectable<T>
    class JsonSerializer<T>
    {
    public:

        void fromJson(const boost::json::value& jv, T& obj,
            const awl::bitmap<JsonSerializationOptions> options = {})
        {
            const boost::json::object& jo = asObject(jv);

            if (options[JsonSerializationOptions::RejectUnknownFields])
            {
                const auto& names = T::member_names();

                for (const auto& member : jo)
                {
                    const std::string key(member.key());

                    if (names.find(key) == names.end())
                    {
                        throw JsonException(std::format("Unknown JSON field '{}'.", key));
                    }
                }
            }

            awl::for_each_index(obj.as_tuple(), [&obj, &jo, options](auto& field_val, size_t index)
            {
                using FieldType = std::decay_t<decltype(field_val)>;
                JsonSerializer<FieldType> formatter;
                const std::string& key = obj.member_names()[index];
                const auto i = jo.find(key);
                const boost::json::value null_jv;
                const boost::json::value& field_jv = i != jo.end() ? i->value() : null_jv;

                try
                {
                    formatter.fromJson(field_jv, field_val, options);
                }
                catch (const JsonException& e)
                {
                    throw JsonException(e, { field_jv.kind(), type_hint<FieldType>(), key });
                }
            });
        }

        void toJson(const T& obj, boost::json::value& jv)
        {
            boost::json::object jo;

            awl::for_each_index(obj.as_const_tuple(), [&obj, &jo](auto& field_val, size_t index)
            {
                JsonSerializer<std::decay_t<decltype(field_val)>> formatter;
                boost::json::value field_jv;
                formatter.toJson(field_val, field_jv);
                jo[obj.member_names()[index]] = std::move(field_jv);
            });

            jv = std::move(jo);
        }
    };
}
