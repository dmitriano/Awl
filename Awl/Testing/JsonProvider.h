#pragma once

#include "BoostExtras/Json/Json.h"

#include "Awl/Testing/AttributeProvider.h"
#include "Awl/StringFormat.h"

namespace awl::testing
{
    class JsonProvider
    {
    public:

        JsonProvider() = default;

        explicit JsonProvider(boost::json::object jo) : _jo(std::move(jo)) {}

        template <class T>
        bool tryGet(const char* name, T& val)
        {
            auto i = _jo.find(name);

            JsonSerializer<T> serializer;

            if (i != _jo.end())
            {
                serializer.fromJson(i->value(), val);

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

            boost::json::value jv;
            serializer.toJson(val, jv);

            _jo[name] = std::move(jv);

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

        boost::json::object _jo;

        bool _dirty = false;
    };

    static_assert(attribute_provider<JsonProvider>);
}
