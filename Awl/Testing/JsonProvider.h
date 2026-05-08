#pragma once

#include "QtExtras/Json/Json.h"

#include "Awl/Testing/AttributeProvider.h"
#include "Awl/StringFormat.h"

namespace awl::testing
{
    class JsonProvider
    {
    public:

        JsonProvider(QJsonObject& jo) : _jo(jo) {}

        template <class T>
        bool tryGet(const char* name, T& val)
        {
            auto i = _jo.find(name);

            JsonSerializer<T> serializer;

            if (i != _jo.end())
            {
                serializer.fromJson(*i, val);

                return true;
            }

            return false;
        }

        template <class T>
        void set(const char* name, const T& val)
        {
            if (_jo.contains(name))
            {
                throw JsonException(std::format(_T("Attribute '{}' is already set."), awl::fromACString(name)));
            }

            JsonSerializer<T> serializer;

            QJsonValue jv;

            serializer.toJson(val, jv);

            _jo[name] = jv;

            _dirty = true;
        }

        // Quick fix. Clear the attributes from the previous test.
        void clear()
        {
            _jo = {};
        }

        bool isDirty() const
        {
            return _dirty;
        }

    private:

        QJsonObject& _jo;

        bool _dirty = false;
    };

    static_assert(attribute_provider<JsonProvider>);
}
