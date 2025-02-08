#pragma once

#include "Awl/String.h"
#include "Awl/EnumTraits.h"

#include <string>

#include <QString>

namespace qtil
{
    inline void ConvertString(std::wstring& to, const QString& from)
    {
        to = from.toStdWString();
    }

    inline void ConvertString(std::string& to, const QString& from)
    {
        to = from.toStdString();
    }

    inline void ConvertString(QString& to, const std::wstring& from)
    {
        to = QString::fromStdWString(from);
    }

    inline void ConvertString(QString& to, std::string& from)
    {
        to = QString::fromStdString(from);
    }

    inline void ConvertString(QString& to, const QString& from)
    {
        to = from;
    }

    inline void ConvertString(std::string& to, const std::string& from)
    {
        to = from;
    }

    inline void ConvertString(std::wstring& to, const std::wstring& from)
    {
        to = from;
    }

    template <class C>
    inline const std::string& FromQString(const std::basic_string<C>& val)
    {
        return val;
    }

    inline awl::String FromQString(const QString & q_string)
    {
        awl::String str;

        ConvertString(str, q_string);

        return str;
    }

    inline QString ToQString(const QString& from)
    {
        return from;
    }

    inline QString ToQString(const std::string & from)
    {
        return QString::fromStdString(from);
    }

    //on Windows platform with Unicode we theoretically can do QString::fromStdWString(name)
    inline QString ToQString(const std::wstring & from)
    {
        return QString::fromStdWString(from);
    }
}
