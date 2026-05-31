#pragma once

#include <functional>

namespace awl::coro
{
    class IDispatcher
    {
    public:

        virtual ~IDispatcher() = default;

        virtual void post(std::move_only_function<void()> func) = 0;

        virtual void join() = 0;
    };
}
