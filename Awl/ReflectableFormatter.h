/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Reflection.h"
#include "Awl/Separator.h"
#include "Awl/StringFormat.h"

#include <concepts>
#include <cstddef>
#include <format>
#include <ios>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>

namespace std
{
    template <awl::reflectable T, class CharT>
    struct formatter<T, CharT> : formatter<std::basic_string<CharT>, CharT>
    {
        template <class FormatContext>
        auto format(const T& val, FormatContext& ctx) const
        {
            const auto tuple = awl::object_as_tuple(val);
            std::basic_ostringstream<CharT> out;

            out << static_cast<CharT>('{');

            awl::basic_separator<CharT> sep(static_cast<CharT>(','));

            awl::for_each_index(tuple, [&out, &sep](const auto& field, auto index)
            {
                out << sep;

                // GCC 13.3 requires a string literal and does not compile this.
                // static constexpr auto format_string = awl::string_from_ascii<CharT>("{}={}");
                // out << sep << std::format(format_string, T::member_names()[index], field);

                // So we can't avoid code duplication with std::format.
                if constexpr (std::same_as<CharT, char>)
                {
                    out << std::format("{}={}", T::member_names()[index], field);
                }
                else if constexpr (std::same_as<CharT, wchar_t>)
                {
                    out << std::format(L"{}={}", T::member_names()[index], field);
                }
            });

            out << static_cast<CharT>('}');

            return formatter<std::basic_string<CharT>, CharT>::format(out.str(), ctx);
        }
    };
}
