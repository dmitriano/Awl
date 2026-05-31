#pragma once

#include "Awl/Coro/IExecutor.h"

#include <functional>

namespace awl::testing
{
    class FakeDispatcher : public awl::coro::IDispatcher
    {
    public:

        void post(std::move_only_function<void()> func) override
        {
            func();
        }

        void join() override
        {}
    };
}
