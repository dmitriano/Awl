#pragma once

#include "Awl/Logger.h"

#include <memory>
#include <string>
#include <utility>

namespace awl
{
    template <class Derived, class Base>
    class LoggerFactoryImpl : public Base
    {
    public:
        std::shared_ptr<Logger> createLogger(std::string source) const override
        {
            return static_cast<const Derived&>(*this).m_logger->createLogger(std::move(source));
        }
    };
}
