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

        awl::ostream& _out;
        std::string _minLevel;
        std::size_t _minSeverity;
        std::vector<std::string> _source;
    };
}
