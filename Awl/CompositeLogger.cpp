#include "Awl/CompositeLogger.h"

namespace awl
{
    void CompositeLogger::subscribe(LoggerObserver* p_logger)
    {
        _loggers.subscribe(p_logger);
    }

    void CompositeLogger::unsubscribe(LoggerObserver* p_logger)
    {
        _loggers.unsubscribe(p_logger);
    }

    bool CompositeLogger::enabled(const std::string& level) const
    {
        return _loggers.notifyUntil(&ILogger::enabled, level);
    }

    void CompositeLogger::doLog(const std::string& level, const LogString& message)
    {
        _loggers.forEach([&](ILogger& logger)
        {
            logger.log(level, message);
        });
    }

    std::shared_ptr<ILogger> CompositeLogger::createLogger(std::string source) const
    {
        static_cast<void>(source);
        return nullptr;
    }
}
