#pragma once

#include "QtExtras/Json/JsonSerializer.h"
#include "QtExtras/StringConversion.h"

#include "Awl/TypeTraits.h"
#include "Awl/VectorSet.h"
#include "Awl/ObservableSet.h"
#include "Awl/StringFormat.h"
#include "Awl/Mp/TypeDescriptor.h"

#include <vector>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <type_traits>

namespace awl
{
    namespace helpers
    {
        template <class Container>
        struct inserter : std::false_type {};

        template <class Container> requires (
            awl::is_specialization_v<Container, std::vector> ||
            awl::is_specialization_v<Container, std::deque> ||
            awl::is_specialization_v<Container, std::list>)
        struct inserter<Container> : std::true_type
        {
            static void reserve(Container& v, size_t n)
            {
                v.reserve(n);
            }

            static void insert(Container& v, typename Container::value_type&& val)
            {
                v.push_back(std::move(val));
            }
        };

        template <class Container> requires (
            awl::is_specialization_v<Container, awl::vector_set> ||
            awl::is_specialization_v<Container, awl::observable_set> ||
            awl::is_specialization_v<Container, std::set> ||
            awl::is_specialization_v<Container, std::multiset> ||
            awl::is_specialization_v<Container, std::unordered_set> ||
            awl::is_specialization_v<Container, std::unordered_multiset>)
        struct inserter<Container> : std::true_type
        {
            static void reserve(Container& set, size_t n)
            {
                static_cast<void>(set);
                static_cast<void>(n);
            }

            static void insert(Container& set, typename Container::value_type&& val)
            {
                set.insert(std::move(val));
            }
        };
    }

    template <class Container>
        requires helpers::inserter<Container>::value
    class JsonSerializer<Container>
    {
    public:

        using value_type = Container;
        using T = typename Container::value_type;

        void FromJson(const QJsonValue& jv, value_type& v)
        {
            EnsureType(jv, QJsonValue::Array);
            QJsonArray ja = jv.toArray();
            helpers::inserter<Container>::reserve(v, static_cast<size_t>(ja.size()));
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

                helpers::inserter<Container>::insert(v, std::move(val));

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
