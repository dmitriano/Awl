#pragma once

#include "QtExtras/Json/Json.h"
#include "QtExtras/Json/JsonException.h"
#include "QtExtras/Json/JsonRange.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

namespace awl
{
    template <class T>
    void FromJson(const QJsonValue & jv, T & val)
    {
        JsonSerializer<T>().FromJson(jv, val);
    }

    template <class T>
    void ToJson(const T & val, QJsonValue & jv)
    {
        JsonSerializer<T>().ToJson(val, jv);
    }

    template <class T>
    QJsonValue ToJson(const T& val)
    {
        QJsonValue jv;

        JsonSerializer<T>().ToJson(val, jv);

        return jv;
    }

    template <class Struct>
    void StructFromString(const QJsonDocument& jdoc, Struct& val)
    {
        QJsonValue jval;

        if (jdoc.isObject())
        {
            QJsonObject jobj = jdoc.object();
            jval = jobj;
        }
        else if (jdoc.isArray())
        {
            QJsonArray ja = jdoc.array();
            jval = ja;
        }
        else
        {
            throw JsonException(_T("The document is empty."));
        }

        FromJson(jval, val);
    }

    template <class Struct>
    void StructFromString(const QString &message, Struct & val)
    {
        QJsonDocument jdoc = QJsonDocument::fromJson(message.toUtf8());

        StructFromString(jdoc, val);
    }

    inline void MergeJsonObjects(QJsonObject& to, const QJsonObject& from)
    {
        for (auto it = from.constBegin(); it != from.constEnd(); ++it)
        {
            to.insert(it.key(), it.value());
        }
    }

    inline QString JsonToString(const QJsonDocument& jdoc)
    {
        const QByteArray bytes = jdoc.toJson();

        return QString::fromUtf8(bytes);
    }

    inline QString JsonToString(const QJsonObject& jo)
    {
        return JsonToString(QJsonDocument(jo));
    }

    inline QString JsonToString(const QJsonArray& ja)
    {
        return JsonToString(QJsonDocument(ja));
    }

    inline QString JsonToString(const QJsonValue& jv)
    {
        if (jv.isObject())
        {
            return JsonToString(jv.toObject());
        }
        else if (jv.isArray())
        {
            return JsonToString(jv.toArray());
        }
        else
        {
            throw JsonException(_T("The document is empty."));
        }
    }

    template <class Struct>
    QString StructToString(const Struct& val)
    {
        QJsonValue jv = ToJson(val);

        return JsonToString(jv);
    }
}
