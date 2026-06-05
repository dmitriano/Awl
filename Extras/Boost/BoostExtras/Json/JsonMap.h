#pragma once

#include "BoostExtras/Json/JsonHelpers.h"
#include "BoostExtras/Json/JsonSerializer.h"
#include "BoostExtras/Json/TypeHint.h"

#include "Awl/TypeTraits.h"

#include <string>
#include <type_traits>

namespace awl
{
    template <class Container> requires awl::insertable_map<Container> &&
        std::is_same_v<typename Container::key_type, std::string>
    class JsonSerializer<Container>
    {
    public:

        using value_type = Container;
        using T = typename Container::mapped_type;
        using pair = typename Container::value_type;

        void fromJson(const boost::json::value& jv, value_type& map)
        {
            const boost::json::object& jo = asObject(jv);
            JsonSerializer<T> formatter;
            map.clear();

            for (const auto& item : jo)
            {
                T val;
                const boost::json::value& item_jv = item.value();

                try
                {
                    formatter.fromJson(item_jv, val);
                }
                catch (JsonException& e)
                {
                    e.append({ item_jv.kind(), type_hint<T>(), std::string(item.key()) });
                    throw;
                }

                const bool new_key = map.insert(pair(std::string(item.key()), std::move(val))).second;

                if (!new_key)
                {
                    throw JsonException(std::format(_T("Duplicate map key {}."), std::string(item.key())));
                }
            }
        }

        void toJson(const value_type& map, boost::json::value& jv)
        {
            boost::json::object jo;
            JsonSerializer<T> formatter;

            for (const auto& item : map)
            {
                boost::json::value item_jv;
                formatter.toJson(item.second, item_jv);
                jo[item.first] = std::move(item_jv);
            }

            jv = std::move(jo);
        }
    };
}
