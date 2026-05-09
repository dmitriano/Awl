#pragma once

#include "Awl/ILogger.h"

#include <memory>
#include <string>
#include <utility>

namespace awl
{
    template <class... Interfaces>
    class LoggerFactoryImpl : public Interfaces...
    {
    public:

        explicit LoggerFactoryImpl(std::shared_ptr<ILogger> logger) :
            _logger(std::move(logger))
        {}

        std::shared_ptr<ILogger> createLogger(std::string source) const override
        {
            return logger()->createLogger(std::move(source));
        }

    protected:

        const std::shared_ptr<ILogger>& logger() const
        {
            return _logger;
        }

    private:

        const std::shared_ptr<ILogger> _logger;
    };
}
