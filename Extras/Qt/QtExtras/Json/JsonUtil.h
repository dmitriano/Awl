/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
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
    void fromJson(const QJsonValue& jv, T& val)
    {
        JsonSerializer<T>().fromJson(jv, val);
    }

    template <class T>
    void toJson(const T& val, QJsonValue& jv)
    {
        JsonSerializer<T>().toJson(val, jv);
    }

    template <class T>
    QJsonValue toJson(const T& val)
    {
        QJsonValue jv;

        JsonSerializer<T>().toJson(val, jv);

        return jv;
    }

    template <class Struct>
    void structFromString(const QJsonDocument& jdoc, Struct& val)
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

        fromJson(jval, val);
    }

    template <class Struct>
    void structFromString(const QString& message, Struct& val)
    {
        QJsonDocument jdoc = QJsonDocument::fromJson(message.toUtf8());

        structFromString(jdoc, val);
    }

    inline void mergeJsonObjects(QJsonObject& to, const QJsonObject& from)
    {
        for (auto it = from.constBegin(); it != from.constEnd(); ++it)
        {
            to.insert(it.key(), it.value());
        }
    }

    inline QString jsonToString(const QJsonDocument& jdoc)
    {
        const QByteArray bytes = jdoc.toJson();

        return QString::fromUtf8(bytes);
    }

    inline QString jsonToString(const QJsonObject& jo)
    {
        return jsonToString(QJsonDocument(jo));
    }

    inline QString jsonToString(const QJsonArray& ja)
    {
        return jsonToString(QJsonDocument(ja));
    }

    inline QString jsonToString(const QJsonValue& jv)
    {
        if (jv.isObject())
        {
            return jsonToString(jv.toObject());
        }
        else if (jv.isArray())
        {
            return jsonToString(jv.toArray());
        }
        else
        {
            throw JsonException(_T("The document is empty."));
        }
    }

    template <class Struct>
    QString structToString(const Struct& val)
    {
        QJsonValue jv = toJson(val);

        return jsonToString(jv);
    }
}
