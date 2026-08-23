/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ITimeQueue.h"

namespace awl::testing
{
    class TimeQueueAwaitable
    {
    public:

        TimeQueueAwaitable(ITimeQueue& executor, std::chrono::nanoseconds delay) :
            _executor(executor),
            _delay(delay)
        {}

        bool await_ready() const noexcept
        {
            return _delay <= std::chrono::nanoseconds(0);
        }

        void await_suspend(std::coroutine_handle<> handle)
        {
            _executor.executeAfter(handle, _delay);
        }

        void await_resume() const noexcept
        {}

    private:

        ITimeQueue& _executor;
        std::chrono::nanoseconds _delay;
    };
}
