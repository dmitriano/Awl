/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Logger.h"
#include "Awl/StdConsole.h"

#include <memory>
#include <string>
#include <vector>

namespace awl
{
    class ConsoleLogger : public Logger
    {
    public:

        using Logger::log;

        ConsoleLogger(
            awl::ostream& out = awl::cout(),
            std::string min_level = LogLevel::Debug,
            std::string source = {});

        bool enabled(const std::string& level) const override;

        void log(const std::string& level, const LogString& message) override;

        std::shared_ptr<Logger> createLogger(std::string source) const override;

    private:

        ConsoleLogger(
            awl::ostream& out,
            std::string min_level,
            std::vector<std::string> source);

        awl::ostream& m_out;
        std::string m_minLevel;
        std::size_t m_minSeverity;
        std::vector<std::string> m_source;
    };
}
