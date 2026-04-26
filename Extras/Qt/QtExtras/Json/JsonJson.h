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

        void fromJson(const QJsonValue & jv, QJsonValue & val)
        {
            val = jv;
        }

        void toJson(const QJsonValue & val, QJsonValue & jv)
        {
            jv = val;
        }
    };

    template <>
    class JsonSerializer<QJsonObject>
    {
    public:

        void fromJson(const QJsonValue& jv, QJsonObject& val)
        {
            EnsureType(jv, QJsonValue::Object);
            
            val = jv.toObject();
        }

        void toJson(const QJsonObject& val, QJsonValue& jv)
        {
            jv = val;
        }
    };

    template <>
    class JsonSerializer<QJsonArray>
    {
    public:

        void fromJson(const QJsonValue& jv, QJsonArray& val)
        {
            EnsureType(jv, QJsonValue::Array);

            val = jv.toArray();
        }

        void toJson(const QJsonObject& val, QJsonValue& jv)
        {
            jv = val;
        }
    };
}
