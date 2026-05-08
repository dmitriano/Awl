/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/ILoggerFactory.h"
#include "Awl/String.h"
#include "Awl/LogFormat.h"
#include "Awl/LogLevel.h"
#include "Awl/LogString.h"
#include "Awl/StringFormat.h"

#include <format>
#include <type_traits>
#include <utility>

namespace awl
{
    class Logger : public ILoggerFactory
    {
    public:

        virtual bool enabled(const std::string& level) const
        {
            static_cast<void>(level);
            return true;
        }

        virtual void log(const std::string& level, const LogString& message) = 0;

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void log(const std::string& level, std::type_identity_t<LogFormat<char, Args...>> fmt, Args&&... args)
        {
            logFormatted<char>(level, fmt, std::forward<Args>(args)...);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void log(const std::string& level, std::type_identity_t<LogFormat<wchar_t, Args...>> fmt, Args&&... args)
        {
            logFormatted<wchar_t>(level, fmt, std::forward<Args>(args)...);
        }

        void debug(const LogString& message)
        {
            logLasy(LogLevel::Debug, message);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void debug(std::type_identity_t<LogFormat<char, Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Debug, fmt, std::forward<Args>(args)...);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void debug(std::type_identity_t<LogFormat<wchar_t, Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Debug, fmt, std::forward<Args>(args)...);
        }

        void trace(const LogString& message)
        {
            logLasy(LogLevel::Trace, message);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void trace(std::type_identity_t<LogFormat<char, Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Trace, fmt, std::forward<Args>(args)...);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void trace(std::type_identity_t<LogFormat<wchar_t, Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Trace, fmt, std::forward<Args>(args)...);
        }

        void info(const LogString& message)
        {
            logLasy(LogLevel::Info, message);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void info(std::type_identity_t<LogFormat<char, Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Info, fmt, std::forward<Args>(args)...);
        }
        
        template <class... Args>
            requires (sizeof...(Args) > 0)
        void info(std::type_identity_t<LogFormat<wchar_t, Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Info, fmt, std::forward<Args>(args)...);
        }

        void warning(const LogString& message)
        {
            logLasy(LogLevel::Warning, message);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void warning(std::type_identity_t<LogFormat<char, Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Warning, fmt, std::forward<Args>(args)...);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void warning(std::type_identity_t<LogFormat<wchar_t, Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Warning, fmt, std::forward<Args>(args)...);
        }

        void error(const LogString& message)
        {
            logLasy(LogLevel::Error, message);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void error(std::type_identity_t<LogFormat<char, Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Error, fmt, std::forward<Args>(args)...);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void error(std::type_identity_t<LogFormat<wchar_t, Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Error, fmt, std::forward<Args>(args)...);
        }

        void critical(const LogString& message)
        {
            logLasy(LogLevel::Critical, message);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void critical(std::type_identity_t<LogFormat<char, Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Critical, fmt, std::forward<Args>(args)...);
        }

        template <class... Args>
            requires (sizeof...(Args) > 0)
        void critical(std::type_identity_t<LogFormat<wchar_t, Args...>> fmt, Args&&... args)
        {
            log(LogLevel::Critical, fmt, std::forward<Args>(args)...);
        }

    private:

        template <class Character, class... Args>
            requires (sizeof...(Args) > 0)
        void logFormatted(const std::string& level, std::type_identity_t<LogFormat<Character, Args...>> fmt, Args&&... args)
        {
            if (enabled(level))
            {
                log(level, LogString(std::format(fmt.format(), std::forward<Args>(args)...), fmt.location()));
            }
        }

        void logLasy(const std::string& level, const LogString& message)
        {
            if (enabled(level))
            {
                log(level, message);
            }
        }
    };
}
