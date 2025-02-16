/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"
#include "QtExtras/Json/TypeHint.h"
#include "QtExtras/StringConversion.h"

#include "Awl/TypeTraits.h"
#include "Awl/StringFormat.h"

#include <ranges>

namespace awl
{
    template <class Container> requires insertable_map<Container> &&
        (std::is_same_v<typename Container::key_type, std::string> || std::is_same_v<typename Container::key_type, QString>)
    class JsonSerializer<Container>
    {
    private:

        using value_type = Container;
        using T = typename Container::mapped_type;

        using pair = typename Container::value_type;

    public:

        void FromJson(const QJsonValue& jv, value_type& map)
        {
            EnsureType(jv, QJsonValue::Object);
            QJsonObject jo = jv.toObject();
            JsonSerializer<T> formatter;

            map.clear();

            for (QJsonObject::const_iterator i = jo.begin(); i != jo.end(); ++i)
            {
                T val;
                const QJsonValue & jvv = i.value();

                try
                {
                    formatter.FromJson(jvv, val);
                }
                catch (JsonException& e)
                {
                    e.append({ jvv.type(), type_hint<T>(), i.key().toLatin1().data() });

                    throw e;
                }

                using MapKey = typename Container::key_type;
                MapKey map_key;
                
                awl::ConvertString(map_key, i.key());
                
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

                QString key = awl::ToQString(p.first);
                
                jo[key] = jv_val;
            }

            jv = jo;
        }
    };
}
