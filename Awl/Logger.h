/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/EnumTraits.h"

namespace awl
{
    AWL_SEQUENTIAL_ENUM(LogLevel,
        Debug,
        Trace,
        Info,
        Warning,
        Error
    )
}

AWL_ENUM_TRAITS(awl, LogLevel)

namespace awl
{
    class Logger
    {
    public:

        virtual ~Logger() = default;

        virtual void log(LogLevel level, String message) = 0;

        void debug(String message)
        {
            log(LogLevel::Debug, message);
        }

        void trace(String message)
        {
            log(LogLevel::Trace, message);
        }

        void info(String message)
        {
            log(LogLevel::Info, message);
        }
        
        void warning(String message)
        {
            log(LogLevel::Warning, message);
        }

        void error(String message)
        {
            log(LogLevel::Error, message);
        }
    };
}
