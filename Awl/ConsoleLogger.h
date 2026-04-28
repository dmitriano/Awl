/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Logger.h"
#include "Awl/StdConsole.h"
#include "Awl/Time.h"
#include "Awl/String.h"

#include <chrono>
#include <string_view>

#if defined(__APPLE__)
#include <ctime>
#endif

namespace awl
{
    class ConsoleLogger : public Logger
    {
    public:

        using Logger::log;

        ConsoleLogger(awl::ostream& out = awl::cout()) : m_out(out) {}

        void log(const std::string& level, const LogString& message) override
        {
            awl::ostringstream temp_out;
            const std::source_location location = message.location();

            // A quick fix to prevent compiler error "‘to_chars’ is unavailable: introduced in iOS 16.3"
            // with IPHONEOS_DEPLOYMENT_TARGET = 14.0 on Apple platform.
            // Alternatively IPHONEOS_DEPLOYMENT_TARGET can be set to 16.6, for example.
#if defined(__APPLE__)
            auto tp = std::chrono::system_clock::now();
            const std::time_t t = std::chrono::system_clock::to_time_t(tp);
            temp_out << std::ctime(&t);
#else
            temp_out << std::chrono::system_clock::now();
#endif

            temp_out << _T('\t')
                     << level
                     << _T('\t')
                     << awl::fromAString(std::string(fileName(location.file_name())))
                     << _T(':')
                     << location.line()
                     << _T('\t')
                     << message.str()
                     << _T('\n');

            m_out << temp_out.str();
        }

    private:

        static std::string_view fileName(const char* path)
        {
            const std::string_view full_path(path);
            const size_t pos = full_path.find_last_of("/\\");
            return pos == std::string_view::npos ? full_path : full_path.substr(pos + 1);
        }

        awl::ostream& m_out;
    };
}
