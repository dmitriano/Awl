/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/EnumTraits.h"
#include "Awl/StringFormat.h"

#include <format>
#include <string>
namespace std
{
    template <awl::sequential_enum T, class CharT>
    struct formatter<T, CharT> : formatter<std::string, CharT>
    {
        template <class FormatContext>
        auto format(T val, FormatContext& ctx) const
        {
            return formatter<std::string, CharT>::format(awl::enum_to_string(val), ctx);
        }
    };
}
