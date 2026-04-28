/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
namespace std
{
    template <>
    struct formatter<std::string, wchar_t> : formatter<wstring_view, wchar_t>
    {
        auto format(const std::string& val, wformat_context& ctx) const
        {
            const std::wstring text = awl::decodeString(val.c_str());
            return formatter<wstring_view, wchar_t>::format(text, ctx);
        }
    };

    template <>
    struct formatter<std::wstring, char> : formatter<string_view, char>
    {
        auto format(const std::wstring& val, format_context& ctx) const
        {
            const std::string text = awl::encodeString(val.c_str());
            return formatter<string_view, char>::format(text, ctx);
        }
    };

#ifdef AWL_QT

    template <>
    struct formatter<QString, char> : formatter<string_view, char>
    {
        auto format(const QString& val, format_context& ctx) const
        {
            const std::string text = val.toStdString();
            return formatter<string_view, char>::format(text, ctx);
        }
    };

    template <>
    struct formatter<QString, wchar_t> : formatter<wstring_view, wchar_t>
    {
        auto format(const QString& val, wformat_context& ctx) const
        {
            const std::wstring text = val.toStdWString();
            return formatter<wstring_view, wchar_t>::format(text, ctx);
        }
    };

#endif
}
