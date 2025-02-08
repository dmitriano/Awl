#pragma once

#include "QtExtras/Json/JsonException.h"

#include "Awl/StringFormat.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

namespace awl
{
    inline String TypeToString(QJsonValue::Type t)
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

        throw JsonException(format() << _T("Wrong type value: ") << t << _T("."));
    }
        
    inline void EnsureType(const QJsonValue& jv, QJsonValue::Type t)
    {
        if (jv.type() != t)
        {
            throw JsonException(format() << _T("Expected value type: ") << TypeToString(t) << _T(" actul value type: ") << TypeToString(jv.type()));
        }
    }
    
    inline bool IsNull(const QJsonValue& jv)
    {
        return jv.isUndefined() || jv.isNull();
    }
}
