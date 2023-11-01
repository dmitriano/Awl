/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/LogLevel.h"
#include "Awl/LogString.h"

namespace awl
{
    class Logger
    {
    public:

        virtual ~Logger() = default;

        virtual void log(LogLevel level, LogString message) = 0;

        void debug(LogString message)
        {
            log(LogLevel::Debug, message);
        }

        void trace(LogString message)
        {
            log(LogLevel::Trace, message);
        }

        void info(LogString message)
        {
            log(LogLevel::Info, message);
        }
        
        void warning(LogString message)
        {
            log(LogLevel::Warning, message);
        }

        void error(LogString message)
        {
            log(LogLevel::Error, message);
        }
    };
}
