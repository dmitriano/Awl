#pragma once

#include "Awl/Logger.h"

#include <memory>
#include <string>
#include <utility>

namespace awl
{
    template <class Base>
    class LoggerFactoryImpl : public Base
    {
    public:
        explicit LoggerFactoryImpl(std::shared_ptr<Logger> logger) :
            m_logger(std::move(logger))
        {}

        std::shared_ptr<Logger> createLogger(std::string source) const override
        {
            return logger()->createLogger(std::move(source));
        }

    protected:
        const std::shared_ptr<Logger>& logger() const
        {
            return m_logger;
        }

    private:
        const std::shared_ptr<Logger> m_logger;
    };
}
