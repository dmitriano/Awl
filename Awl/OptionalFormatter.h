/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

#include <algorithm>
#include <format>
#include <optional>
#include <string>
namespace std
{
    template <class T, class CharT>
    struct formatter<std::optional<T>, CharT> : formatter<T, CharT>
    {
        template <class FormatContext>
        auto format(const std::optional<T>& val, FormatContext& ctx) const
        {
            if (val)
            {
                return formatter<T, CharT>::format(*val, ctx);
            }

            const std::basic_string<CharT> null_text = awl::string_from_ascii<CharT>("null");

            return std::ranges::copy(null_text, ctx.out()).out;
        }
    };
}
