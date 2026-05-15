#pragma once

#include "Awl/Coro/Awaitable.h"
#include "Awl/Coro/ISharedExecutor.h"

#include <chrono>
#include <memory>

namespace awl::coro
{
    class IAwaitableFactory
    {
    public:

        virtual ~IAwaitableFactory() = default;

        virtual Awaitable<void> sleep(std::chrono::steady_clock::duration delay) = 0;

        virtual std::shared_ptr<ISharedDispatcher> makeSharedExecutor() = 0;
    };
}
