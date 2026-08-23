#pragma once

#include "Awl/Coro/Awaitable.h"

#include <chrono>

namespace awl::coro
{
    class IDelayedExecutor
    {
    public:

        virtual ~IDelayedExecutor() = default;

        virtual Awaitable<void> sleep(std::chrono::steady_clock::duration delay) = 0;
    };
}
