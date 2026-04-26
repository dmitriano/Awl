/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"
#include "QtExtras/StringConversion.h"
#include "QtExtras/Json/TypeHint.h"

#include "Awl/TypeTraits.h"
#include "Awl/StringFormat.h"
#include "Awl/Inserter.h"

#include <type_traits>
#include <ranges>

namespace awl
{
    template <class Container>
        requires inserter_defined<Container>
    class JsonSerializer<Container>
    {
    public:

        using value_type = Container;
        using T = typename Container::value_type;

        void fromJson(const QJsonValue& jv, value_type& v)
        {
            EnsureType(jv, QJsonValue::Array);
            QJsonArray ja = jv.toArray();
            inserter<Container>::reserve(v, static_cast<size_t>(ja.size()));
            JsonSerializer<T> formatter;

            v.clear();

            size_t index = 0;

            for (auto j_elem : ja)
            {
                T val;

                try
                {
                    formatter.fromJson(j_elem, val);
                }
                catch (JsonException& e)
                {
                    e.append({ j_elem.type(), type_hint<T>(), std::to_string(index) });

                    throw e;
                }

                inserter<Container>::insert(v, std::move(val));

                ++index;
            }
        }

        void toJson(const value_type& set, QJsonValue& jv)
        {
            QJsonArray ja;
            JsonSerializer<T> formatter;

            for (const T& elem : set)
            {
                QJsonValue elem_jv;
                formatter.toJson(elem, elem_jv);
                ja.append(elem_jv);
            }

            jv = ja;
        }
    };
}
