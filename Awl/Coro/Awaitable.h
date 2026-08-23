#pragma once

#include "Awl/Coro/IAwaitable.h"

#include <coroutine>
#include <memory>
#include <utility>

namespace awl::coro
{
    template <class T>
    class Awaitable
    {
    public:

        explicit Awaitable(std::unique_ptr<IAwaitable<T>> impl) :
            _impl(std::move(impl))
        {}

        bool await_ready() const noexcept
        {
            return _impl->await_ready();
        }

        void await_suspend(std::coroutine_handle<> h)
        {
            _impl->await_suspend(h);
        }

        T await_resume()
        {
            return _impl->await_resume();
        }

    private:

        std::unique_ptr<IAwaitable<T>> _impl;
    };
}
