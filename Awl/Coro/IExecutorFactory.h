#pragma once

#include "Awl/Coro/IDelayedExecutor.h"
#include "Awl/Coro/ISharedExecutor.h"

#include <memory>

namespace awl::coro
{
    class IExecutorFactory
    {
    public:

        virtual ~IExecutorFactory() = default;

        virtual std::shared_ptr<IDelayedExecutor> makeDelayedExecutor() = 0;

        virtual std::shared_ptr<ISharedDispatcher> makeSharedExecutor() = 0;
    };
}
