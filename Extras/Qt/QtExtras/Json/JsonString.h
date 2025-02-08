#pragma once

#include "Qtil/Json/JsonSerializer.h"

namespace qtil
{
    template <>
    class JsonSerializer<QString>
    {
    public:

        void FromJson(const QJsonValue & jv, QString & val)
        {
            EnsureType(jv, QJsonValue::String);
            val = jv.toString();
        }

        void ToJson(const QString & val, QJsonValue & jv)
        {
            jv = val;
        }
    };

    template <>
    class JsonSerializer<std::string>
    {
    public:

        void FromJson(const QJsonValue & jv, std::string & val)
        {
            EnsureType(jv, QJsonValue::String);
            val = jv.toString().toStdString();
        }

        void ToJson(const std::string & val, QJsonValue & jv)
        {
            jv = QString::fromStdString(val);
        }
    };

    template <>
    class JsonSerializer<std::wstring>
    {
    public:

        void FromJson(const QJsonValue & jv, std::wstring & val)
        {
            EnsureType(jv, QJsonValue::String);
            val = jv.toString().toStdWString();
        }

        void ToJson(const std::wstring & val, QJsonValue & jv)
        {
            jv = QString::fromStdWString(val);
        }
    };
}
