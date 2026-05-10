/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/FormatterUtil.h"

#include <boost/multiprecision/number.hpp>
#include <format>
#include <string_view>

namespace std
{
    template <class Backend, boost::multiprecision::expression_template_option ExpressionTemplates, class CharT>
    struct formatter<boost::multiprecision::number<Backend, ExpressionTemplates>, CharT> : formatter<std::basic_string_view<CharT>, CharT>
    {
        template <class FormatContext>
        auto format(const boost::multiprecision::number<Backend, ExpressionTemplates>& val, FormatContext& ctx) const
        {
            const std::basic_string<CharT> text = awl::to_basic_string_via_ostream<CharT>(val);

            return formatter<std::basic_string_view<CharT>, CharT>::format(text, ctx);
        }
    };
}
