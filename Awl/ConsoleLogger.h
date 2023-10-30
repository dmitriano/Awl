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

        void log(Level level, String message) override
        {
            awl::cout() << std::chrono::system_clock::now() << _T('\t') <<
                awl::FromAString(enum_to_string(level)) << _T('\t') << message << std::endl;
        }
    };
}
