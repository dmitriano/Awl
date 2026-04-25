/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/LogFormat.h"
#include "Awl/LogLevel.h"
#include "Awl/LogString.h"

#include <format>
#include <source_location>
#include <type_traits>
#include <utility>

namespace awl
{
    class Logger
    {
    public:

        virtual ~Logger() = default;

        virtual bool enabled(const std::string& level) const
        {
            static_cast<void>(level);
            return true;
        }

        virtual void log(const std::string& level, const LogString& message) = 0;

        virtual void log(const std::string& level, const LogString& message, std::source_location location)
        {
            static_cast<void>(location);
            log(level, message);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void log(const std::string& level, std::type_identity_t<LogFormat<Args...>> fmt, Args&&... args)
        {
            if (enabled(level))
            {
                log(level, LogString(std::format(fmt.format(), std::forward<Args>(args)...)), fmt.location());
            }
        }

        void debug(const LogString& message, std::source_location location = std::source_location::current())
        {
            logLasy(LogLevel::Debug, message, location);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void debug(std::type_identity_t<LogFormat<Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Debug, fmt, std::forward<Args>(args)...);
        }

        void trace(const LogString& message, std::source_location location = std::source_location::current())
        {
            logLasy(LogLevel::Trace, message, location);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void trace(std::type_identity_t<LogFormat<Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Trace, fmt, std::forward<Args>(args)...);
        }

        void info(const LogString& message, std::source_location location = std::source_location::current())
        {
            logLasy(LogLevel::Info, message, location);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void info(std::type_identity_t<LogFormat<Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Info, fmt, std::forward<Args>(args)...);
        }
        
        void warning(const LogString& message, std::source_location location = std::source_location::current())
        {
            logLasy(LogLevel::Warning, message, location);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void warning(std::type_identity_t<LogFormat<Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Warning, fmt, std::forward<Args>(args)...);
        }

        void error(const LogString& message, std::source_location location = std::source_location::current())
        {
            logLasy(LogLevel::Error, message, location);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void error(std::type_identity_t<LogFormat<Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Error, fmt, std::forward<Args>(args)...);
        }

    private:

        void logLasy(const std::string& level, const LogString& message, std::source_location location)
        {
            if (enabled(level))
            {
                log(level, message, location);
            }
        }
    };
}
