#pragma once

#include "QtExtras/Json/Json.h"

#include "Awl/Testing/AttributeProvider.h"
#include "Awl/StringFormat.h"

namespace awl::testing
{
    class JsonProvider
    {
    public:

        JsonProvider(const QJsonObject& jo) : m_jo(jo) {}

        template <class T>
        bool TryGet(const char* name, T& val)
        {
            auto i = m_jo.find(name);

            JsonSerializer<T> serializer;

            if (i != m_jo.end())
            {
                serializer.FromJson(*i, val);

                return true;
            }

            return false;
        }

        template <class T>
        void Set(const char* name, const T& val)
        {
            if (m_jo.contains(name))
            {
                throw JsonException(awl::format() << "Attribute '" << name << "' is already set.");
            }

            JsonSerializer<T> serializer;

            QJsonValue jv;

            serializer.ToJson(val, jv);

            m_jo[name] = jv;
        }

    private:

        const QJsonObject& m_jo;
    };

    static_assert(attribute_provider<JsonProvider>);
}
