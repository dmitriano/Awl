#pragma once

#include "BoostExtras/Json/JsonHelpers.h"
#include "BoostExtras/Json/JsonSerializer.h"
#include "BoostExtras/Json/TypeHint.h"

#include "Awl/Inserter.h"

#include <ranges>
#include <string>
#include <type_traits>

namespace awl::boost_json
{
    template <class Container>
        requires awl::inserter_defined<Container>
    class JsonSerializer<Container>
    {
    public:

        using value_type = Container;
        using T = typename Container::value_type;

        void fromJson(const boost::json::value& jv, value_type& v)
        {
            const boost::json::array& ja = asArray(jv);
            awl::inserter<Container>::reserve(v, ja.size());
            JsonSerializer<T> formatter;
            v.clear();

            size_t index = 0;

            for (const boost::json::value& elem_jv : ja)
            {
                T val;

                try
                {
                    formatter.fromJson(elem_jv, val);
                }
                catch (JsonException& e)
                {
                    e.append({ elem_jv.kind(), type_hint<T>(), std::to_string(index) });
                    throw;
                }

                awl::inserter<Container>::insert(v, std::move(val));
                ++index;
            }
        }

        void toJson(const value_type& v, boost::json::value& jv)
        {
            boost::json::array ja;
            JsonSerializer<T> formatter;

            for (const T& elem : v)
            {
                boost::json::value elem_jv;
                formatter.toJson(elem, elem_jv);
                ja.push_back(std::move(elem_jv));
            }

            jv = std::move(ja);
        }
    };
}
