#pragma once

#include "Awl/Coro/Task.h"
#include "Awl/Coro/TaskSink.h"
#include "Awl/Observable.h"

#include <coroutine>
#include <exception>
#include <utility>

namespace awl::coro
{
    class Job;

    namespace detail
    {
        struct JobPromise : Observable<TaskSink>, PromiseContext
        {
            // Job is async result of our coroutine
            // it is created before execution of the coroutine body
            // it can be either co_awaited inside another coroutine
            // or used via special interface for extracting values (is_ready and get)
            Job get_return_object();

            std::suspend_always initial_suspend() noexcept
            {
                return {};
            }

            void unhandled_exception() noexcept;

            // when final suspend is executed 'value' is already set
            // we need to suspend this coroutine in order to use value in other coroutine or through 'get' function
            // otherwise promise object would be destroyed (together with stored value) and one couldn't access Job result
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
                    //std::coroutine_handle<> await_suspend(std::coroutine_handle<JobPromise> h) noexcept
                    //{
                    //    JobPromise& promise = h.promise();

                    //    return promise._awaitingCoroutine ? promise._awaitingCoroutine : std::noop_coroutine();
                    //}
                    void await_suspend(std::coroutine_handle<JobPromise> h) noexcept
                    {
                        JobPromise& promise = h.promise();

                        // The Promise is always owned by Job,
                        // so we do not call h.destroy() here.
                        if (promise._awaitingCoroutine)
                        {
                            promise.resumeAwaiting();
                        }
                        else
                        {
                            promise.notify(&TaskSink::onFinished);
                        }
                    }

                    void await_resume() noexcept {}
                };

                return transfer_awaitable{};
            }

            void return_void() {}

            template<typename T>
            auto await_transform(Task<T>& task) noexcept
            {
                task._h.promise().setDispatcherIfEmpty(_dispatcher);

                return task.await(_dispatcher);
            }

            template<typename T>
            auto await_transform(Task<T>&& task) noexcept
            {
                task._h.promise().setDispatcherIfEmpty(_dispatcher);

                return std::move(task).await(_dispatcher);
            }

            auto await_transform(Job& job) noexcept;

            auto await_transform(Job&& job) noexcept;

            template<class Awaitable>
            Awaitable& await_transform(Awaitable& awaitable) noexcept
            {
                setAwaitableDispatcher(awaitable, _dispatcher);

                return awaitable;
            }

            template<class Awaitable>
            Awaitable&& await_transform(Awaitable&& awaitable) noexcept
            {
                setAwaitableDispatcher(awaitable, _dispatcher);

                return std::move(awaitable);
            }
        };
    }
}
