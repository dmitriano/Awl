/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "Awl/String.h"
#include "Awl/EnumTraits.h"

#include <QString>

#include <string>

namespace awl
{
    inline void convertString(std::wstring& to, const QString& from)
    {
        to = from.toStdWString();
    }

    inline void convertString(std::string& to, const QString& from)
    {
        to = from.toStdString();
    }

    inline void convertString(QString& to, const std::wstring& from)
    {
        to = QString::fromStdWString(from);
    }

    inline void convertString(QString& to, std::string& from)
    {
        to = QString::fromStdString(from);
    }

    inline void convertString(QString& to, const QString& from)
    {
        to = from;
    }

    inline void convertString(std::string& to, const std::string& from)
    {
        to = from;
    }

    inline void convertString(std::wstring& to, const std::wstring& from)
    {
        to = from;
    }

    template <class C>
    inline const std::string& fromQString(const std::basic_string<C>& val)
    {
        return val;
    }

    inline awl::String fromQString(const QString & q_string)
    {
        awl::String str;

        convertString(str, q_string);

        return str;
    }

    inline QString toQString(const QString& from)
    {
        return from;
    }

    inline QString toQString(const std::string & from)
    {
        return QString::fromStdString(from);
    }

    //on Windows platform with Unicode we theoretically can do QString::fromStdWString(name)
    inline QString toQString(const std::wstring & from)
    {
        return QString::fromStdWString(from);
    }
}
