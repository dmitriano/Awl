/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Logger.h"
#include "Awl/StdConsole.h"
#include "Awl/Time.h"

#include <chrono>

namespace awl
{
    class ConsoleLogger : public Logger
    {
    public:

        ConsoleLogger(awl::ostream& out = awl::cout()) : m_out(out) {}

        void log(LogLevel level, String message) override
        {
            m_out << std::chrono::system_clock::now() << _T('\t') <<
                awl::FromAString(enum_to_string(level)) << _T('\t') << message << std::endl;
        }

    private:

        awl::ostream& m_out;
    };
}
