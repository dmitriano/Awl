/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/LogLevel.h"
#include "Awl/LogString.h"

#include <type_traits>
#include <utility>

namespace awl
{
    class Logger
    {
    public:

        template <class... Args>
        using format_string = std::basic_format_string<Char, std::type_identity_t<Args>...>;

        virtual ~Logger() = default;

        virtual bool enabled(const std::string& level) const
        {
            static_cast<void>(level);
            return true;
        }

        virtual void log(const std::string& level, const LogString& message) = 0;

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void log(const std::string& level, format_string<Args...> fmt, Args&&... args)
        {
            if (enabled(level))
            {
                log(level, LogString(std::format(fmt, std::forward<Args>(args)...)));
            }
        }

        void debug(const LogString& message)
        {
            logLasy(LogLevel::Debug, message);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void debug(format_string<Args...> fmt, Args&&... args)
        {
            log(LogLevel::Debug, fmt, std::forward<Args>(args)...);
        }

        void trace(const LogString& message)
        {
            logLasy(LogLevel::Trace, message);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void trace(format_string<Args...> fmt, Args&&... args)
        {
            log(LogLevel::Trace, fmt, std::forward<Args>(args)...);
        }

        void info(const LogString& message)
        {
            logLasy(LogLevel::Info, message);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void info(format_string<Args...> fmt, Args&&... args)
        {
            log(LogLevel::Info, fmt, std::forward<Args>(args)...);
        }
        
        void warning(const LogString& message)
        {
            logLasy(LogLevel::Warning, message);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void warning(format_string<Args...> fmt, Args&&... args)
        {
            log(LogLevel::Warning, fmt, std::forward<Args>(args)...);
        }

        void error(const LogString& message)
        {
            logLasy(LogLevel::Error, message);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void error(format_string<Args...> fmt, Args&&... args)
        {
            log(LogLevel::Error, fmt, std::forward<Args>(args)...);
        }

    private:

        void logLasy(const std::string& level, const LogString& message)
        {
            if (enabled(level))
            {
                log(level, message);
            }
        }
    };
}
