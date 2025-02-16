#pragma once

#include "Awl/Exception.h"

#include "Awl/StringFormat.h"
#include "QtExtras/Json/JsonUtil.h"
#include "QtExtras/Json/JsonException.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>

namespace awl
{
    inline QJsonDocument loadDocumentFromFile(QString file_name)
    {
        QFile file(file_name);

        if (!file.open(QFile::ReadOnly))
        {
            throw JsonException(awl::format() << _T("Cannot open input file '") << file.fileName() << "'.");
        }

        QByteArray content = file.readAll();

        QJsonDocument document = QJsonDocument::fromJson(content);

        return document;
    }

    inline QJsonArray loadArrayFromFile(QString file_name)
    {
        QJsonDocument document = loadDocumentFromFile(file_name);

        if (!document.isArray())
        {
            throw JsonException(awl::format() << _T("JSON array expected in file '") << file_name << "'.");
        }

        QJsonArray ja = document.array();

        return ja;
    }

    inline QJsonObject loadObjectFromFile(QString file_name)
    {
        QJsonDocument document = loadDocumentFromFile(file_name);

        if (!document.isObject())
        {
            throw JsonException(awl::format() << _T("JSON object expected in file '") << file_name << "'.");
        }

        QJsonObject jo = document.object();

        return jo;
    }

    template <class Struct>
    void StructFromFile(QString file_name, Struct& val)
    {
        QJsonDocument jdoc = loadDocumentFromFile(file_name);

        StructFromString(jdoc, val);
    }

    inline void saveDocumentToFile(QString file_name, const QJsonDocument& document)
    {
        QFile file(file_name);

        if (!file.open(QFile::WriteOnly))
        {
            throw JsonException(awl::format() << _T("Cannot open file '") << file.fileName() << "' for writing.");
        }

        file.write(document.toJson());
    }

    inline void saveObjectToFile(QString file_name, const QJsonObject& jo)
    {
        saveDocumentToFile(file_name, QJsonDocument(jo));
    }

    inline void saveArrayToFile(QString file_name, const QJsonArray& ja)
    {
        saveDocumentToFile(file_name, QJsonDocument(ja));
    }

    template <class Struct>
    void saveStructToFile(QString file_name, const Struct& val)
    {
        QJsonValue jv = ToJson(val);

        QJsonDocument jdoc;

        switch (jv.type())
        {
        case QJsonValue::Object:
            jdoc.setObject(jv.toObject());
            break;
        case QJsonValue::Array:
            jdoc.setArray(jv.toArray());
            break;
        case QJsonValue::Null:
        case QJsonValue::Undefined:
            break;
        default:
            throw JsonException(awl::format() << "Can't create a document form json value of type " << TypeToString(jv.type()));
        }

        saveDocumentToFile(file_name, jdoc);
    }
}
