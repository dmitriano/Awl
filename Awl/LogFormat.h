/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

#include <concepts>
#include <format>
#include <source_location>
#include <type_traits>

namespace awl
{
    template <class Character, class... Args>
    class LogFormat
    {
    public:

        using format_string = std::basic_format_string<Character, std::type_identity_t<Args>...>;

        consteval LogFormat(format_string fmt, std::source_location location = std::source_location::current()) noexcept :
            m_fmt(fmt),
            m_location(location)
        {
        }

        template <class T>
            requires std::constructible_from<format_string, const T&>
        consteval LogFormat(const T& fmt, std::source_location location = std::source_location::current()) noexcept :
            m_fmt(fmt),
            m_location(location)
        {
        }

        constexpr format_string format() const noexcept
        {
            return m_fmt;
        }

        constexpr std::source_location location() const noexcept
        {
            return m_location;
        }

    private:

        format_string m_fmt;
        std::source_location m_location;
    };
}
