/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Logger.h"
#include "Awl/StdConsole.h"

#include <string>

namespace awl
{
    class ConsoleLogger : public Logger
    {
    public:

        using Logger::log;

        ConsoleLogger(
            awl::ostream& out = awl::cout(),
            std::string min_level = LogLevel::Debug);

        bool enabled(const std::string& level) const override;

        void log(const std::string& level, const LogString& message) override;

    private:

        awl::ostream& m_out;
        std::size_t m_minSeverity;
    };
}
