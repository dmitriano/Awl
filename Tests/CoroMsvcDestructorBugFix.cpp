/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/TimeQueue.h"
#include "Awl/StringFormat.h"

#include <coroutine>
#include <memory>
#include <optional>

namespace
{
    //template <bool owning>
    struct JobPromise;

    //template <bool owning>
    struct Job
    {
        // declare promise type
        using promise_type = JobPromise;

        Job(std::coroutine_handle<promise_type> handle);

        Job(const Job&) = delete;
        
        Job(Job&& other);

        Job& operator = (const Job&) = delete;

        Job& operator = (const Job&& other);

        ~Job();

        std::coroutine_handle<promise_type> handle;
        std::shared_ptr<awl::ILogger> logger;
    };

    struct JobPromise
    {
        JobPromise(awl::testing::TestContext context, awl::coro::IDelayedExecutor&) :
            _logger(std::move(context.logger))
        {}

        std::coroutine_handle<> _awaitingCoroutine;
        std::shared_ptr<awl::ILogger> _logger;

        Job get_return_object();

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
                // always stop at final suspend
                bool await_ready() noexcept
                {
                    return false;
                }

                // resume awaiting coroutine or if there is no coroutine to resume return special coroutine that do
                // nothing
                //std::coroutine_handle<> await_suspend(std::coroutine_handle<JobPromise> h) noexcept
                //{
                //    std::coroutine_handle<> val = _awaitingCoroutine ? _awaitingCoroutine : std::noop_coroutine();

                //    h.destroy();

                //    return val;
                //}

                //if await_suspend returns void, control is immediately returned to the caller/resumer of the current coroutine (this coroutine remains suspended), otherwise
                //if await_suspend returns bool,
                //the value true returns control to the caller/resumer of the current coroutine
                //the value false resumes the current coroutine.
                //if await_suspend returns a coroutine handle for some other coroutine, that handle is resumed (by a call to handle.resume())
                //(note this may chain to eventually cause the current coroutine to resume)
                void await_suspend(std::coroutine_handle<JobPromise> h) noexcept
                {
                    auto coro = h.promise()._awaitingCoroutine;
                    
                    h.destroy();

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

        auto await_transform(awl::coro::DelayedAwaitable awaitable)
        {
            return awaitable;
        }

        // also we can await other Job<T>
        auto await_transform(const Job& update_task)
        {
            if (!update_task.handle)
            {
                throw std::runtime_error("coroutine without promise awaited");
            }

            if (update_task.handle.promise()._awaitingCoroutine)
            {
                throw std::runtime_error("coroutine already awaited");
            }

            struct task_awaitable
            {
                std::coroutine_handle<JobPromise> handle;

                // check if this Job already has value computed
                bool await_ready()
                {
                    return handle.done();
                }

                // h - is a handle to coroutine that calls co_await
                // store coroutine handle to be resumed after computing Job value
                void await_suspend(std::coroutine_handle<> h)
                {
                    handle.promise()._awaitingCoroutine = h;
                }

                // when ready return value to a consumer
                auto await_resume()
                {}
            };

            return task_awaitable{ update_task.handle };
        }
    };

    inline Job JobPromise::get_return_object()
    {
        return { std::coroutine_handle<JobPromise>::from_promise(*this) };
    }

    inline Job::Job(std::coroutine_handle<promise_type> handle) :
        handle(handle),
        logger(handle.promise()._logger)
    {
        logger->debug("Job constructor.");
    }

    inline Job::Job(Job&& other) :
        handle(other.handle),
        logger(std::move(other.logger))
    {
        logger->debug("Job move constructor.");
    }

    inline Job& Job::operator = (const Job&& other)
    {
        handle = other.handle;
        logger = other.logger;

        logger->debug("Job move assignment.");

        return *this;
    }

    inline Job::~Job()
    {
        logger->debug("Job destructor.");
    }

    // example

    using namespace std::chrono_literals;

    Job TestTimerAwait(awl::testing::TestContext context, awl::coro::IDelayedExecutor& delayed_executor)
    {
        using namespace std::chrono_literals;

        context.logger->debug(_T("TestTimerAwait started."));

        co_await awl::coro::DelayedAwaitable(delayed_executor, 1s);

        context.logger->debug(_T("TestTimerAwait finished."));
    }

    Job TestNestedTask(awl::testing::TestContext context, awl::coro::IDelayedExecutor& delayed_executor)
    {
        using namespace std::chrono_literals;

        context.logger->debug(_T("TestNestedTask started."));

        auto task = TestTimerAwait(context, delayed_executor);

        co_await awl::coro::DelayedAwaitable(delayed_executor, 2s);

        context.logger->debug(_T("Time interval has elapsed."));

        //We can't wait for a destroyed task.
        //co_await task;

        context.logger->debug(_T("TestNestedTask finished."));
    }
}

//The correct logger output ends with two destructor calls:
//Job constructor.
//TestNestedTask started.
//Job constructor.
//TestTimerAwait started.
//TestTimerAwait finished.
//Time interval has elapsed.
//TestNestedTask finished.
//Job destructor.
//Job destructor.
AWL_UNSTABLE_EXAMPLE(CoroMsvcDestructorBugFix)
{
    awl::testing::TimeQueue time_queue;

    auto task = TestNestedTask(context, time_queue);

    time_queue.loop();
}
