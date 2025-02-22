/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"

namespace awl
{
    //Can be used like std::vector<QJsonValue>.
    template <>
    class JsonSerializer<QJsonValue>
    {
    public:

        void FromJson(const QJsonValue & jv, QJsonValue & val)
        {
            val = jv;
        }

        void ToJson(const QJsonValue & val, QJsonValue & jv)
        {
            jv = val;
        }
    };

    template <>
    class JsonSerializer<QJsonObject>
    {
    public:

        void FromJson(const QJsonValue& jv, QJsonObject& val)
        {
            EnsureType(jv, QJsonValue::Object);
            
            val = jv.toObject();
        }

        void ToJson(const QJsonObject& val, QJsonValue& jv)
        {
            jv = val;
        }
    };

    template <>
    class JsonSerializer<QJsonArray>
    {
    public:

        void FromJson(const QJsonValue& jv, QJsonArray& val)
        {
            EnsureType(jv, QJsonValue::Array);

            val = jv.toArray();
        }

        void ToJson(const QJsonObject& val, QJsonValue& jv)
        {
            jv = val;
        }
    };
}
