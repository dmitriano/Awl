/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/FormatterUtil.h"

#include <boost/asio/ip/basic_endpoint.hpp>
#include <format>
#include <string_view>
namespace std
{
    template <class Protocol, class CharT>
    struct formatter<boost::asio::ip::basic_endpoint<Protocol>, CharT> : formatter<std::basic_string_view<CharT>, CharT>
    {
        template <class FormatContext>
        auto format(const boost::asio::ip::basic_endpoint<Protocol>& val, FormatContext& ctx) const
        {
            const std::basic_string<CharT> text = awl::to_basic_string_via_ostream<CharT>(val);

            return formatter<std::basic_string_view<CharT>, CharT>::format(text, ctx);
        }
    };
}
