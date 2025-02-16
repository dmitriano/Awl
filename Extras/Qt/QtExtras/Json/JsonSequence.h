#pragma once

#include "QtExtras/Json/JsonSerializer.h"
#include "QtExtras/StringConversion.h"

#include "Awl/TypeTraits.h"
#include "Awl/StringFormat.h"
#include "Awl/Mp/TypeDescriptor.h"
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

        void FromJson(const QJsonValue& jv, value_type& v)
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
                    formatter.FromJson(j_elem, val);
                }
                catch (JsonException& e)
                {
                    e.append({ j_elem.type(), mp::make_type_name<T>(), awl::aformat() << index });

                    throw e;
                }

                inserter<Container>::insert(v, std::move(val));

                ++index;
            }
        }

        void ToJson(const value_type& set, QJsonValue& jv)
        {
            QJsonArray ja;
            JsonSerializer<T> formatter;

            for (const T& elem : set)
            {
                QJsonValue elem_jv;
                formatter.ToJson(elem, elem_jv);
                ja.append(elem_jv);
            }

            jv = ja;
        }
    };
}
