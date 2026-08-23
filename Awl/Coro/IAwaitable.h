#pragma once

#include <coroutine>

namespace awl::coro
{
    template <class T>
    class IAwaitable
    {
    public:

        virtual ~IAwaitable() = default;

        virtual bool await_ready() const noexcept = 0;

        virtual void await_suspend(std::coroutine_handle<> h) = 0;

        virtual T await_resume() = 0;
    };
}
