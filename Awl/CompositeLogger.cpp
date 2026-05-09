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

    void CompositeLogger::log(const std::string& level, const LogString& message)
    {
        _loggers.notify(
            static_cast<void (ILogger::*)(const std::string&, const LogString&)>(&ILogger::log),
            level,
            message);
    }

    std::shared_ptr<ILogger> CompositeLogger::createLogger(std::string source) const
    {
        static_cast<void>(source);
        return nullptr;
    }
}
