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

namespace awl
{
    namespace detail
    {
        template <class CharT, reflectable T, class Field, class Index>
        void appendReflectableField(std::basic_ostream<CharT>& out, const Field& field, Index index, awl::basic_separator<CharT>& sep)
        {
            out << sep;

            static constexpr auto format_string = awl::string_from_ascii<CharT>("{}={}");

            out << std::format(format_string, T::member_names()[index], field);
        }
    }

    template <class CharT, reflectable T>
    std::basic_ostream<CharT>& writeReflectable(std::basic_ostream<CharT>& out, const T& val)
    {
        const auto tuple = object_as_tuple(val);

        out << static_cast<CharT>('{');

        awl::basic_separator<CharT> sep(static_cast<CharT>(','));

        awl::for_each_index(tuple, [&out, &sep](const auto& field, auto index)
        {
            detail::appendReflectableField<CharT, T>(out, field, index, sep);
        });

        out << static_cast<CharT>('}');

        return out;
    }
}

namespace std
{
    template <awl::reflectable T, class CharT>
    struct formatter<T, CharT> : formatter<std::basic_string<CharT>, CharT>
    {
        template <class FormatContext>
        auto format(const T& val, FormatContext& ctx) const
        {
            std::basic_ostringstream<CharT> out;
            awl::writeReflectable(out, val);

            return formatter<std::basic_string<CharT>, CharT>::format(out.str(), ctx);
        }
    };
}
