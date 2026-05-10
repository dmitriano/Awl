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
            _message(awl::fromACString(m)),
            _location(location)
        {}

        LogString(const wchar_t* m, std::source_location location = std::source_location::current()) :
            _message(awl::fromWCString(m)),
            _location(location)
        {}

        LogString(std::string message, std::source_location location = std::source_location::current()) :
            _message(awl::fromAString(std::move(message))),
            _location(location)
        {}

        LogString(std::wstring message, std::source_location location = std::source_location::current()) :
            _message(awl::fromWString(std::move(message))),
            _location(location)
        {}

        const String& str() const noexcept
        {
            return _message;
        }

        constexpr std::source_location location() const noexcept
        {
            return _location;
        }

    private:

        awl::String _message;
        std::source_location _location;
    };
}
