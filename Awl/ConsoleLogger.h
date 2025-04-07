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

namespace awl
{
    class ConsoleLogger : public Logger
    {
    public:

        ConsoleLogger(awl::ostream& out = awl::cout()) : m_out(out) {}

        void log(std::string level, LogString message) override
        {
            m_out << std::chrono::system_clock::now() << _T('\t') <<
                level << _T('\t') << message.str() << std::endl;
        }

    private:

        awl::ostream& m_out;
    };
}
