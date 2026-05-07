/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Coro/IDelayedExecutor.h"

namespace awl::coro
{
    class DelayedAwaitable
    {
    public:

        DelayedAwaitable(IDelayedExecutor& executor, std::chrono::nanoseconds delay) :
            m_executor(executor),
            m_delay(delay)
        {
        }

        bool await_ready() const noexcept
        {
            return m_delay <= std::chrono::nanoseconds(0);
        }

        void await_suspend(std::coroutine_handle<> handle)
        {
            m_executor.executeAfter(handle, m_delay);
        }

        void await_resume() const noexcept
        {
        }

    private:

        IDelayedExecutor& m_executor;
        std::chrono::nanoseconds m_delay;
    };
}
