/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/ILogger.h"
#include "Awl/Observable.h"

#include <memory>
#include <string>

namespace awl
{
    class CompositeLogger : public ILogger
    {
    public:

        using ILogger::log;
        using LoggerObserver = Observer<ILogger>;

        void subscribe(LoggerObserver* p_logger);

        void unsubscribe(LoggerObserver* p_logger);

        bool enabled(const std::string& level) const override;

        void log(const std::string& level, const LogString& message) override;

        std::shared_ptr<ILogger> createLogger(std::string source) const override;

    private:

        mutable Observable<ILogger, CompositeLogger> _loggers;
    };
}
