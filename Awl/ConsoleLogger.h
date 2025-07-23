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

#if defined(__APPLE__)
#include <ctime>
#endif

namespace awl
{
    class ConsoleLogger : public Logger
    {
    public:

        ConsoleLogger(awl::ostream& out = awl::cout()) : m_out(out) {}

        void log(std::string level, LogString message) override
        {
            // A quick fix to prevent compiler error "‘to_chars’ is unavailable: introduced in iOS 16.3"
            // with IPHONEOS_DEPLOYMENT_TARGET = 14.0 on Apple platform.
            // Alternatively IPHONEOS_DEPLOYMENT_TARGET can be set to 16.6, for example.
#if defined(__APPLE__)
            auto tp = std::chrono::system_clock::now();
            const std::time_t t = std::chrono::system_clock::to_time_t(tp);
            m_out << std::ctime(&t);
#else
            m_out << std::chrono::system_clock::now();
#endif

            m_out << _T('\t') << level << _T('\t') << message.str() << std::endl;
        }

    private:

        awl::ostream& m_out;
    };
}
