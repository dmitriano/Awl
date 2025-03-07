#pragma once

#include <coroutine>
#include <exception>
#include <utility>

#include "Awl/Coro/TaskSink.h"
#include "Awl/Observable.h"

namespace awl
{
    class UpdateTask;

    struct UpdatePromise : Observable<TaskSink>
    {
        // corouine that awaiting this coroutine value
        // we need to store it in order to resume it later when value of this coroutine will be computed
        std::coroutine_handle<> m_awaitingCoroutine;

        // UpdateTask is async result of our coroutine
        // it is created before execution of the coroutine body
        // it can be either co_awaited inside another coroutine
        // or used via special interface for extracting values (is_ready and get)
        UpdateTask get_return_object();

        // there are two kinds of coroutines:
        // 1. eager - that start its execution immediately
        // 2. lazy - that start its execution only after 'co_await'ing on them
        // here I used eager coroutine UpdateTask
        // eager: do not suspend before running coroutine body
        std::suspend_never initial_suspend() noexcept
        {
            return {};
        }

        void unhandled_exception() noexcept;

        // when final suspend is executed 'value' is already set
        // we need to suspend this coroutine in order to use value in other coroutine or through 'get' function
        // otherwise promise object would be destroyed (together with stored value) and one couldn't access UpdateTask result
        // value
        auto final_suspend() noexcept
        {
            // if there is a coroutine that is awaiting on this coroutine resume it
            struct transfer_awaitable
            {
                // always stop at final suspend
                bool await_ready() noexcept
                {
                    return false;
                }

                // resume awaiting coroutine or if there is no coroutine to resume return special coroutine that do
                // nothing
                //std::coroutine_handle<> await_suspend(std::coroutine_handle<UpdatePromise> h) noexcept
                //{
                //    UpdatePromise& promise = h.promise();

                //    return promise.m_awaitingCoroutine ? promise.m_awaitingCoroutine : std::noop_coroutine();
                //}
                void await_suspend(std::coroutine_handle<UpdatePromise> h) noexcept
                {
                    UpdatePromise& promise = h.promise();

                    auto coro = promise.m_awaitingCoroutine;

                    // The Promise is always owned by UpdateTask,
                    // so we do not call h.destroy() here.
                    promise.Notify(&TaskSink::OnFinished);

                    if (coro)
                    {
                        coro.resume();
                    }
                }

                void await_resume() noexcept {}
            };
            
            return transfer_awaitable{};
        }

        void return_void() {}
    };
}
