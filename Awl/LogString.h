/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

#include <source_location>
#include <utility>

namespace awl
{
#ifdef AWL_QT

    class LogString
    {
    public:

        LogString(const char* m, std::source_location location = std::source_location::current()) :
            m_message(m),
            m_location(location)
        {
        }

        LogString(const wchar_t* m, std::source_location location = std::source_location::current()) :
            m_message(QString::fromWCharArray(m)),
            m_location(location)
        {
        }

        LogString(std::string message, std::source_location location = std::source_location::current()) :
            m_message(QString::fromStdString(message)),
            m_location(location)
        {
        }

        LogString(std::wstring message, std::source_location location = std::source_location::current()) :
            m_message(QString::fromStdWString(message)),
            m_location(location)
        {
        }

        LogString(QString message, std::source_location location = std::source_location::current()) :
            m_message(std::move(message)),
            m_location(location)
        {
        }

        const QString& str() const
        {
            return m_message;
        }

        QString& str()
        {
            return m_message;
        }

        constexpr std::source_location location() const noexcept
        {
            return m_location;
        }

    private:

        // Make the Logger compatible with QDebug.
        QString m_message;
        std::source_location m_location;
    };

#else

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
            m_message(awl::fromAString(message)),
            m_location(location)
        {
        }

        LogString(std::wstring message, std::source_location location = std::source_location::current()) :
            m_message(awl::fromWString(message)),
            m_location(location)
        {
        }

        String& str()
        {
            return m_message;
        }

        const String& str() const
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

#endif //AWL_QT
}
