#include "Awl/CompositeLogger.h"

#include <algorithm>
#include <functional>
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

    bool CompositeLogger::removeLogger(const std::shared_ptr<ILogger>& logger)
    {
        auto i = std::ranges::find(_loggers, logger);

        if (i == _loggers.end())
        {
            return false;
        }

        _loggers.erase(i);

        return true;
    }

    bool CompositeLogger::enabled(const std::string& level) const
    {
        using namespace std::placeholders;

        return std::ranges::any_of(
            _loggers,
            std::bind(&ILogger::enabled, _1, std::cref(level)));
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
        if (source.empty())
        {
            return std::const_pointer_cast<ILogger>(shared_from_this());
        }
        else
        {
            std::vector<std::shared_ptr<ILogger>> child_loggers;
            child_loggers.reserve(_loggers.size());

            using namespace std::placeholders;

            std::ranges::transform(
                _loggers,
                std::back_inserter(child_loggers),
                std::bind(&ILogger::createLogger, _1, std::cref(source)));

            return std::make_shared<CompositeLogger>(std::move(child_loggers));
        }
    }
}
