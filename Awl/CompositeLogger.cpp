#include "Awl/CompositeLogger.h"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <utility>

namespace awl
{
    CompositeLogger::CompositeLogger(std::vector<std::shared_ptr<ILogger>> loggers) :
        _loggers(std::move(loggers))
    {}

    void CompositeLogger::addLogger(std::shared_ptr<ILogger> logger)
    {
        _loggers.push_back(std::move(logger));
    }

    bool CompositeLogger::enabled(const std::string& level) const
    {
        return std::ranges::any_of(_loggers, [&](const std::shared_ptr<ILogger>& logger)
        {
            return logger->enabled(level);
        });
    }

    void CompositeLogger::doLog(const std::string& level, const LogString& message)
    {
        std::ranges::for_each(_loggers, [&](const std::shared_ptr<ILogger>& logger)
        {
            logger->log(level, message);
        });
    }

    std::shared_ptr<ILogger> CompositeLogger::createLogger(std::string source) const
    {
        std::vector<std::shared_ptr<ILogger>> child_loggers;
        child_loggers.reserve(_loggers.size());

        std::ranges::transform(_loggers, std::back_inserter(child_loggers), [&](const std::shared_ptr<ILogger>& logger)
        {
            return logger->createLogger(source);
        });

        return std::make_shared<CompositeLogger>(std::move(child_loggers));
    }
}
