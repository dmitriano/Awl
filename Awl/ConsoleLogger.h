/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ILogger.h"
#include "Awl/StdConsole.h"

#include <memory>
#include <string>
#include <vector>

namespace awl
{
    class ConsoleLogger : public ILogger
    {
    public:

        ConsoleLogger(
            awl::ostream& out = awl::cout(),
            std::string min_level = LogLevel::Debug,
            std::string source = {});

        bool enabled(const std::string& level) const override;

        std::shared_ptr<ILogger> createLogger(std::string source) const override;

    protected:

        void doLog(const std::string& level, const LogString& message) override;

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
