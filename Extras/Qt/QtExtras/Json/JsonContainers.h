#pragma once

#include "Qtil/Json/JsonSerializer.h"

#include "Qtil/StringConversion.h"

#include "Awl/TypeTraits.h"
#include "Awl/VectorSet.h"
#include "Awl/ObservableSet.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>

namespace qtil
{
    template <class Container>
    class JsonSerializer<Container, std::enable_if_t<
        (std::is_same_v<typename Container::key_type, std::string> || std::is_same_v<typename Container::key_type, QString>) &&
        (awl::is_specialization_v<Container, std::unordered_map> || awl::is_specialization_v<Container, std::map>)>>
    {
    private:

        using value_type = Container;
        using T = typename Container::mapped_type;

        using pair = typename Container::value_type;

    public:

        void FromJson(const QJsonValue & jv, value_type & map)
        {
            EnsureType(jv, QJsonValue::Object);
            QJsonObject jo = jv.toObject();
            JsonSerializer<T> formatter;

            map.clear();

            for (QJsonObject::const_iterator i = jo.begin(); i != jo.end(); ++i)
            {
                T val;
                const QJsonValue & jvv = i.value();
                formatter.FromJson(jvv, val);

                using MapKey = typename Container::key_type;
                MapKey map_key;
                
                qtil::ConvertString(map_key, i.key());
                
                const bool new_key = map.insert(pair(map_key, val)).second;
                
                if (!new_key)
                {
                    throw JsonException(awl::format() << _T("Duplicate map key ") << i.key().data() << _T("."));
                }
            }
        }

        void ToJson(const value_type & map, QJsonValue & jv)
        {
            QJsonObject jo;
            JsonSerializer<T> formatter;

            for (const auto & p : map)
            {
                QJsonValue jv_val;
                formatter.ToJson(p.second, jv_val);

                QString key = qtil::ToQString(p.first);
                
                jo[key] = jv_val;
            }

            jv = jo;
        }
    };

    template <class Container>
    class JsonSerializer<Container, std::enable_if_t<
        awl::is_specialization_v<Container, std::vector> ||
        awl::is_specialization_v<Container, std::deque> ||
        awl::is_specialization_v<Container, std::list>>>
    {
    public:

        using value_type = Container;
        using T = typename Container::value_type;

        void FromJson(const QJsonValue & jv, value_type & v)
        {
            EnsureType(jv, QJsonValue::Array);
            QJsonArray ja = jv.toArray();
            v.reserve(static_cast<size_t>(ja.size()));
            JsonSerializer<T> formatter;

            v.clear();

            for (auto j_elem : ja)
            {
                T val;
                formatter.FromJson(j_elem, val);
                v.push_back(val);
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

    template <class Container>
    class JsonSerializer<Container, std::enable_if_t<
        awl::is_specialization_v<Container, awl::vector_set> ||
        awl::is_specialization_v<Container, awl::observable_set> ||
        awl::is_specialization_v<Container, std::set> ||
        awl::is_specialization_v<Container, std::multiset> ||
        awl::is_specialization_v<Container, std::unordered_set> ||
        awl::is_specialization_v<Container, std::unordered_multiset>>>
    {
    public:

        using value_type = Container;
        using T = typename Container::value_type;

        void FromJson(const QJsonValue & jv, value_type & v)
        {
            EnsureType(jv, QJsonValue::Array);
            QJsonArray ja = jv.toArray();
            JsonSerializer<T> formatter;

            v.clear();

            for (auto j_elem : ja)
            {
                T val;
                formatter.FromJson(j_elem, val);
                v.insert(std::move(val));
            }
        }

        void ToJson(const value_type & set, QJsonValue& jv)
        {
            QJsonArray ja;
            JsonSerializer<T> formatter;

            for (const T & elem : set)
            {
                QJsonValue elem_jv;
                formatter.ToJson(elem, elem_jv);
                ja.append(elem_jv);
            }

            jv = ja;
        }
    };
}
