/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/String.h"
#include "Awl/Time.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/TimeQueue.h"

#include <coroutine>
#include <optional>

#include <iostream>

namespace
{
    awl::testing::TimeQueue time_queue;
    
    //template <bool owning>
    struct UpdatePromise;

    //template <bool owning>
    struct UpdateTask
    {
        // declare promise type
        using promise_type = UpdatePromise;

        UpdateTask(std::coroutine_handle<promise_type> handle) : 
            handle(handle)
        {
            std::cout << "UpdateTask constructor." << std::endl;
        }

        UpdateTask(const UpdateTask&) = delete;
        
        UpdateTask(UpdateTask&& other) : handle(other.handle)
        {
            std::cout << "UpdateTask move constructor." << std::endl;
        }

        UpdateTask& operator = (const UpdateTask&) = delete;

        UpdateTask& operator = (const UpdateTask&& other)
        {
            handle = other.handle;

            std::cout << "UpdateTask move assignment." << std::endl;

            return *this;
        }

        ~UpdateTask()
        {
            std::cout << "UpdateTask destructor." << std::endl;
        }

        std::coroutine_handle<promise_type> handle;
    };

    struct UpdatePromise
    {
        std::coroutine_handle<> awaiting_coroutine;

        UpdateTask get_return_object();

        std::suspend_never initial_suspend()
        {
            return {};
        }

        void unhandled_exception()
        {
            std::terminate();
        }

        auto final_suspend() noexcept
        {
            // if there is a coroutine that is awaiting on this coroutine resume it
            struct transfer_awaitable
            {
                std::coroutine_handle<> awaiting_coroutine;

                // always stop at final suspend
                bool await_ready() noexcept
                {
                    return false;
                }

                // resume awaiting coroutine or if there is no coroutine to resume return special coroutine that do
                // nothing
                //std::coroutine_handle<> await_suspend(std::coroutine_handle<UpdatePromise> h) noexcept
                //{
                //    std::coroutine_handle<> val = awaiting_coroutine ? awaiting_coroutine : std::noop_coroutine();

                //    h.destroy();

                //    return val;
                //}

                //if await_suspend returns void, control is immediately returned to the caller/resumer of the current coroutine (this coroutine remains suspended), otherwise
                //if await_suspend returns bool,
                //the value true returns control to the caller/resumer of the current coroutine
                //the value false resumes the current coroutine.
                //if await_suspend returns a coroutine handle for some other coroutine, that handle is resumed (by a call to handle.resume())
                //(note this may chain to eventually cause the current coroutine to resume)
                void await_suspend(std::coroutine_handle<UpdatePromise> h) noexcept
                {
                    auto coro = awaiting_coroutine;
                    
                    h.destroy();

                    if (coro)
                    {
                        coro.resume();
                    }
                }

                void await_resume() noexcept {}
            };

            return transfer_awaitable{ awaiting_coroutine };
        }

        void return_void() {}

        // use `co_await std::chrono::seconds{n}` to wait specified amount of time
        auto await_transform(std::chrono::milliseconds d)
        {
            return awl::testing::TimeAwaitable{ time_queue, d };
        }

        // also we can await other UpdateTask<T>
        auto await_transform(UpdateTask& update_task)
        {
            if (!update_task.handle)
            {
                throw std::runtime_error("coroutine without promise awaited");
            }

            if (update_task.handle.promise().awaiting_coroutine)
            {
                throw std::runtime_error("coroutine already awaited");
            }

            struct task_awaitable
            {
                std::coroutine_handle<UpdatePromise> handle;

                // check if this UpdateTask already has value computed
                bool await_ready()
                {
                    return handle.done();
                }

                // h - is a handle to coroutine that calls co_await
                // store coroutine handle to be resumed after computing UpdateTask value
                void await_suspend(std::coroutine_handle<> h)
                {
                    handle.promise().awaiting_coroutine = h;
                }

                // when ready return value to a consumer
                auto await_resume()
                {
                }
            };

            return task_awaitable{ update_task.handle };
        }
    };

    inline UpdateTask UpdatePromise::get_return_object()
    {
        return { std::coroutine_handle<UpdatePromise>::from_promise(*this) };
    }

    // example

    using namespace std::chrono_literals;

    UpdateTask TestTimerAwait(awl::testing::TestContext context)
    {
        using namespace std::chrono_literals;

        context.out << _T("testTimerAwait started.") << std::endl;

        co_await 3s;

        context.out << _T("testTimerAwait finished.") << std::endl;
    }

    UpdateTask TestNestedTimerAwait(awl::testing::TestContext context)
    {
        using namespace std::chrono_literals;

        context.out << _T("testNestedTimerAwait started.") << std::endl;

        auto task = TestTimerAwait(context);

        co_await task;

        context.out << _T("testNestedTimerAwait finished.") << std::endl;
    }
}

// main can't be a coroutine and usually need some sort of looper (io_service or timer loop in this example)
AWT_UNSTABLE_EXAMPLE(CoroNonOwningTimer)
{
    auto task = TestNestedTimerAwait(context);

    // execute deferred coroutines
    time_queue.loop();
}
