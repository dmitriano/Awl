/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/ILogger.h"

#include <memory>
#include <string>
#include <vector>

namespace awl
{
    class CompositeLogger : public ILogger
    {
    public:

        CompositeLogger() = default;

        explicit CompositeLogger(std::vector<std::shared_ptr<ILogger>> loggers);

        void addLogger(std::shared_ptr<ILogger> logger);

        bool removeLogger(const std::shared_ptr<ILogger>& logger);

        bool enabled(const std::string& level) const noexcept override;

        std::shared_ptr<ILogger> createLogger(std::string source) const override;

    protected:

        void doLog(const std::string& level, const LogString& message) noexcept override;

    private:

        std::vector<std::shared_ptr<ILogger>> _loggers;
    };
}
