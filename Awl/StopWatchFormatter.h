/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Time.h"

#include <format>
#include <string_view>
namespace std
{
    template <class CharT>
    struct formatter<awl::StopWatch, CharT> : formatter<std::basic_string_view<CharT>, CharT>
    {
        template <class FormatContext>
        auto format(const awl::StopWatch& val, FormatContext& ctx) const
        {
            const std::basic_string<CharT> text = awl::duration_to_string<CharT>(val.elapsedTime());

            return formatter<std::basic_string_view<CharT>, CharT>::format(text, ctx);
        }
    };
}
