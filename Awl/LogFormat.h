/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

#include <format>
#include <source_location>
#include <type_traits>

namespace awl
{
    template <class... Args>
    class LogFormat
    {
    public:

        using format_string = std::basic_format_string<Char, std::type_identity_t<Args>...>;

        constexpr LogFormat(format_string fmt, std::source_location location = std::source_location::current()) noexcept :
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
