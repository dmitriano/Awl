#pragma once

#include "Awl/Logger.h"

#include <memory>
#include <string>
#include <utility>

namespace awl
{
    template <class... Interfaces>
    class LoggerFactoryImpl : public Interfaces...
    {
    public:

        explicit LoggerFactoryImpl(std::shared_ptr<Logger> logger) :
            _logger(std::move(logger))
        {}

        std::shared_ptr<Logger> createLogger(std::string source) const override
        {
            return logger()->createLogger(std::move(source));
        }

    protected:

        const std::shared_ptr<Logger>& logger() const
        {
            return _logger;
        }

    private:

        const std::shared_ptr<Logger> _logger;
    };
}
