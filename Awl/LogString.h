/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

#include <source_location>
#include <string>
#include <utility>

namespace awl
{
    class LogString
    {
    public:

        LogString(const char* m, std::source_location location = std::source_location::current()) :
            m_message(awl::fromACString(m)),
            m_location(location)
        {
        }

        LogString(const wchar_t* m, std::source_location location = std::source_location::current()) :
            m_message(awl::fromWCString(m)),
            m_location(location)
        {
        }

        LogString(std::string message, std::source_location location = std::source_location::current()) :
            m_message(awl::fromAString(std::move(message))),
            m_location(location)
        {
        }

        LogString(std::wstring message, std::source_location location = std::source_location::current()) :
            m_message(awl::fromWString(std::move(message))),
            m_location(location)
        {
        }

        String& str() noexcept
        {
            return m_message;
        }

        const String& str() const noexcept
        {
            return m_message;
        }

        constexpr std::source_location location() const noexcept
        {
            return m_location;
        }

    private:

        awl::String m_message;
        std::source_location m_location;
    };
}
