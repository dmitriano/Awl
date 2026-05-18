/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Reflection.h"
#include "Awl/Separator.h"

#include <concepts>
#include <cstddef>
#include <format>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>

namespace awl
{
    template <class CharT, reflectable T>
    std::basic_ostream<CharT>& operator << (std::basic_ostream<CharT>& out, const T& val);

    namespace detail
    {
        template <class CharT, reflectable T, size_t index, class Tuple>
        void appendReflectableField(std::basic_ostream<CharT>& out, const Tuple& tuple, awl::basic_separator<CharT>& sep)
        {
            out << sep;

            using awl::operator <<;

            out << T::member_names()[index] << static_cast<CharT>('=') << std::get<index>(tuple);
        }

        template <class CharT, reflectable T, size_t... indices>
        std::basic_ostream<CharT>& writeReflectable(std::basic_ostream<CharT>& out, const T& val, std::index_sequence<indices...>)
        {
            const auto tuple = object_as_tuple(val);

            out << static_cast<CharT>('{');

            awl::basic_separator<CharT> sep(static_cast<CharT>(','));
            (appendReflectableField<CharT, T, indices>(out, tuple, sep), ...);

            out << static_cast<CharT>('}');

            return out;
        }
    }

    template <class CharT, reflectable T>
    std::basic_ostream<CharT>& writeReflectable(std::basic_ostream<CharT>& out, const T& val)
    {
        using Tie = typename tuplizable_traits<T>::ConstTie;

        return detail::writeReflectable(out, val, std::make_index_sequence<std::tuple_size_v<Tie>>{});
    }

    template <class CharT, reflectable T>
    std::basic_ostream<CharT>& operator << (std::basic_ostream<CharT>& out, const T& val)
    {
        return writeReflectable(out, val);
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
