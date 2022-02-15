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
#include <chrono>

namespace
{
    awl::testing::TimeQueue time_queue;

    // basic coroutine single-threaded async task example

    template<typename T>
    struct task_promise_type;

    template<typename T>
    struct task;

    template<typename T>
    struct task_promise_type
    {
        // value to be computed
        // when task is not completed (coroutine didn't co_return anything yet) value is empty
        std::optional<T> value;

        // corouine that awaiting this coroutine value
        // we need to store it in order to resume it later when value of this coroutine will be computed
        std::coroutine_handle<> awaiting_coroutine;

        // task is async result of our coroutine
        // it is created before execution of the coroutine body
        // it can be either co_awaited inside another coroutine
        // or used via special interface for extracting values (is_ready and get)
        task<T> get_return_object();

        // there are two kinds of coroutines:
        // 1. eager - that start its execution immediately
        // 2. lazy - that start its execution only after 'co_await'ing on them
        // here I used eager coroutine task
        // eager: do not suspend before running coroutine body
        std::suspend_never initial_suspend()
        {
            return {};
        }

        // store value to be returned to awaiting coroutine or accessed through 'get' function
        void return_value(T val)
        {
            value = std::move(val);
        }

        void unhandled_exception()
        {
            // alternatively we can store current exeption in std::exception_ptr to rethrow it later
            std::terminate();
        }

        // when final suspend is executed 'value' is already set
        // we need to suspend this coroutine in order to use value in other coroutine or through 'get' function
        // otherwise promise object would be destroyed (together with stored value) and one couldn't access task result
        // value
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
                std::coroutine_handle<> await_suspend(std::coroutine_handle<task_promise_type> h) noexcept
                {
                    static_cast<void>(h);

                    // resume awaiting coroutine or if there is no coroutine to resume return special coroutine that do
                    // nothing
                    return awaiting_coroutine ? awaiting_coroutine : std::noop_coroutine();
                }
                void await_resume() noexcept {}
            };
            return transfer_awaitable{ awaiting_coroutine };
        }

        // there are multiple ways to add co_await into coroutines
        // I used `await_transform`

        // use `co_await std::chrono::seconds{n}` to wait specified amount of time
        auto await_transform(std::chrono::nanoseconds d)
        {
            return awl::testing::TimeAwaitable{ time_queue, d };
        }

        // also we can await other task<T>
        template<typename U>
        auto await_transform(task<U>& task)
        {
            if (!task.handle)
            {
                throw std::runtime_error("coroutine without promise awaited");
            }

            if (task.handle.promise().awaiting_coroutine)
            {
                throw std::runtime_error("coroutine already awaited");
            }

            struct task_awaitable
            {
                std::coroutine_handle<task_promise_type<U>> handle;

                // check if this task already has value computed
                bool await_ready()
                {
                    return handle.promise().value.has_value();
                }

                // h - is a handle to coroutine that calls co_await
                // store coroutine handle to be resumed after computing task value
                void await_suspend(std::coroutine_handle<> h)
                {
                    handle.promise().awaiting_coroutine = h;
                }

                // when ready return value to a consumer
                auto await_resume()
                {
                    return std::move(*(handle.promise().value));
                }
            };

            return task_awaitable{ task.handle };
        }
    };

    template<typename T>
    struct task
    {
        // declare promise type
        using promise_type = task_promise_type<T>;

        task(std::coroutine_handle<promise_type> handle) : handle(handle) {}

        task(task&& other) : handle(std::exchange(other.handle, nullptr)) {}

        task& operator=(task&& other)
        {
            if (handle)
            {
                handle.destroy();
            }
            
            handle = other.handle;
        }

        ~task()
        {
            if (handle)
            {
                handle.destroy();
            }
        }

        // interface for extracting value without awaiting on it

        bool is_ready() const
        {
            if (handle) {
                return handle.promise().value.has_value();
            }
            return false;
        }

        T get()
        {
            if (handle) {
                return std::move(*handle.promise().value);
            }
            throw std::runtime_error("get from task without promise");
        }

        std::coroutine_handle<promise_type> handle;
    };

    template<typename T>
    task<T> task_promise_type<T>::get_return_object()
    {
        return { std::coroutine_handle<task_promise_type>::from_promise(*this) };
    }

    // example

    using namespace std::chrono_literals;

    task<int> wait_n(int n)
    {
        std::cout << "before wait " << n << '\n';
        co_await std::chrono::seconds(n);
        std::cout << "after wait " << n << '\n';
        co_return n;
    }

    task<int> test()
    {
        for (auto c : "hello world\n")
        {
            std::cout << c;
            co_await 100ms;
        }

        std::cout << "test step 1\n";
        auto w3 = wait_n(3);
        std::cout << "test step 2\n";
        auto w2 = wait_n(2);
        std::cout << "test step 3\n";
        auto w1 = wait_n(1);
        std::cout << "test step 4\n";
        auto r = co_await w2 + co_await w3;
        std::cout << "awaiting already computed coroutine\n";
        co_return co_await w1 + r;
    }
}

// main can't be a coroutine and usually need some sort of looper (io_service or timer loop in this example)
AWT_EXAMPLE(CoroTimer)
{
    // do something

    auto result = test();

    // execute deferred coroutines
    time_queue.loop();

    context.out << "result: " << result.get();
}
