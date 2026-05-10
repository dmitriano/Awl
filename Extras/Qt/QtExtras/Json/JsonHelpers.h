/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonException.h"

#include "Awl/StringFormat.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
namespace awl
{
    inline String typeToString(QJsonValue::Type t)
    {
        switch (t)
        {
            case QJsonValue::Null: return _T("Null");
            case QJsonValue::Bool: return _T("Bool");
            case QJsonValue::Double: return _T("Double");
            case QJsonValue::String: return _T("String");
            case QJsonValue::Array: return _T("Array");
            case QJsonValue::Object: return _T("Object");
            case QJsonValue::Undefined: return _T("Undefined");
        }

        throw JsonException(std::format(_T("Wrong type value: {}."), static_cast<int>(t)));
    }
        
    inline void ensureType(const QJsonValue& jv, QJsonValue::Type t)
    {
        if (jv.type() != t)
        {
            throw JsonException(std::format(_T("Expected value type: {}, actul value type: {}"), typeToString(t), typeToString(jv.type())));
        }
    }
    
    inline bool isNull(const QJsonValue& jv)
    {
        return jv.isUndefined() || jv.isNull();
    }

    inline QJsonObject asObject(const QJsonValue& jv)
    {
        awl::ensureType(jv, QJsonValue::Object);

        return jv.toObject();
    }

    inline QJsonArray asArray(const QJsonValue& jv)
    {
        awl::ensureType(jv, QJsonValue::Array);

        return jv.toArray();
    }

    inline QString asString(const QJsonValue& jv)
    {
        awl::ensureType(jv, QJsonValue::String);

        return jv.toString();
    }

    // TODO: Define template function.
    // template <typename String>
    inline QJsonObject asObject(const QJsonObject& jo, const char* name)
    {
        QJsonValue jv = jo[name];

        try
        {
            awl::ensureType(jv, QJsonValue::Object);

            return jv.toObject();
        }
        catch (JsonException& e)
        {
            e.append(JsonException::ValueInfo{ jv.type(), "", name });

            throw;
        }
    }

    inline QJsonArray asArray(const QJsonObject& jo, const char* name)
    {
        QJsonValue jv = jo[name];

        try
        {
            awl::ensureType(jv, QJsonValue::Array);

            return jv.toArray();
        }
        catch (JsonException& e)
        {
            e.append(JsonException::ValueInfo{ jv.type(), "", name });

            throw;
        }
    }

    inline QString asString(const QJsonObject& jo, const char* name)
    {
        QJsonValue jv = jo[name];

        try
        {
            awl::ensureType(jv, QJsonValue::String);

            return jv.toString();
        }
        catch (JsonException& e)
        {
            e.append(JsonException::ValueInfo{ jv.type(), "", name });

            throw;
        }
    }
}
