/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"

namespace awl
{
    template <>
    class JsonSerializer<QString>
    {
    public:

        void fromJson(const QJsonValue & jv, QString & val)
        {
            EnsureType(jv, QJsonValue::String);
            val = jv.toString();
        }

        void toJson(const QString & val, QJsonValue & jv)
        {
            jv = val;
        }
    };

    template <>
    class JsonSerializer<std::string>
    {
    public:

        void fromJson(const QJsonValue & jv, std::string & val)
        {
            EnsureType(jv, QJsonValue::String);
            val = jv.toString().toStdString();
        }

        void toJson(const std::string & val, QJsonValue & jv)
        {
            jv = QString::fromStdString(val);
        }
    };

    template <>
    class JsonSerializer<std::wstring>
    {
    public:

        void fromJson(const QJsonValue & jv, std::wstring & val)
        {
            EnsureType(jv, QJsonValue::String);
            val = jv.toString().toStdWString();
        }

        void toJson(const std::wstring & val, QJsonValue & jv)
        {
            jv = QString::fromStdWString(val);
        }
    };
}
