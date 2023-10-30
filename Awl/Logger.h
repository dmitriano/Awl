#pragma once

#include "Awl/String.h"
#include "Awl/EnumTraits.h"

namespace awl
{
    class Logger
    {
    public:

        AWL_SEQUENTIAL_ENUM(Level,
            Debug,
            Trace,
            Info,
            Warning,
            Error
        );

        virtual ~Logger() = default;

        virtual void log(Level level, String message) = 0;

        void debug(String message)
        {
            log(Level::Debug, message);
        }

        void trace(String message)
        {
            log(Level::Trace, message);
        }

        void info(String message)
        {
            log(Level::Info, message);
        }
        
        void warning(String message)
        {
            log(Level::Warning, message);
        }

        void error(String message)
        {
            log(Level::Error, message);
        }
    };

    AWL_ENUM_TRAITS(Logger, Level)
}
