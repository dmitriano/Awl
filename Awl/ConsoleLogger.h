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

        explicit ConsoleLogger(
            std::string source,
            awl::ostream& out = awl::cout(),
            std::string level = LogLevel::Debug);

        bool enabled(const std::string& level) const override;

        std::shared_ptr<ILogger> createLogger(std::string source) const override;

    protected:

        void doLog(const std::string& level, const LogString& message) override;

    private:

        ConsoleLogger(
            std::vector<std::string> source,
            awl::ostream& out,
            std::string level);

        awl::ostream& _out;
        std::string _level;
        std::size_t _severity;
        std::vector<std::string> _source;
    };
}
