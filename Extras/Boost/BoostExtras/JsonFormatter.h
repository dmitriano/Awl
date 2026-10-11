/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

#include <boost/json.hpp>
#include <format>
#include <string>

namespace awl::detail
{
    template <class T, class CharT>
    struct JsonFormatter : std::formatter<std::basic_string<CharT>, CharT>
    {
        template <class FormatContext>
        auto format(const T& val, FormatContext& ctx) const
        {
            const std::string json = boost::json::serialize(val);
            const std::basic_string<CharT> text = awl::StringConvertor<CharT>::convertFrom(json.c_str());
            return std::formatter<std::basic_string<CharT>, CharT>::format(text, ctx);
        }
    };
}

namespace std
{
    template <class CharT>
    struct formatter<boost::json::value, CharT> : awl::detail::JsonFormatter<boost::json::value, CharT> {};

    template <class CharT>
    struct formatter<boost::json::object, CharT> : awl::detail::JsonFormatter<boost::json::object, CharT> {};

    template <class CharT>
    struct formatter<boost::json::array, CharT> : awl::detail::JsonFormatter<boost::json::array, CharT> {};
}
