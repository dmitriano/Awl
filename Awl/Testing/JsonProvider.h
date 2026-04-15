#pragma once

#include "QtExtras/Json/Json.h"

#include "Awl/Testing/AttributeProvider.h"
#include "Awl/StringFormat.h"

namespace awl::testing
{
    class JsonProvider
    {
    public:

        JsonProvider(QJsonObject& jo) : m_jo(jo) {}

        template <class T>
        bool tryGet(const char* name, T& val)
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
        void set(const char* name, const T& val)
        {
            if (m_jo.contains(name))
            {
                throw JsonException(std::format(_T("Attribute '{}' is already set."), awl::fromACString(name)));
            }

            JsonSerializer<T> serializer;

            QJsonValue jv;

            serializer.ToJson(val, jv);

            m_jo[name] = jv;

            m_dirty = true;
        }

        // Quick fix. Clear the attributes from the previous test.
        void clear()
        {
            m_jo = {};
        }

        bool isDirty() const
        {
            return m_dirty;
        }

    private:

        QJsonObject& m_jo;

        bool m_dirty = false;
    };

    static_assert(attribute_provider<JsonProvider>);
}
