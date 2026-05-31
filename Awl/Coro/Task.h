#pragma once

#include "Awl/Coro/IDispatcher.h"

#include <concepts>
#include <coroutine>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace awl::coro
{
    template<typename T>
    struct Task;

    namespace detail
    {
        inline void resume(std::shared_ptr<IDispatcher> dispatcher, std::coroutine_handle<> handle)
        {
            if (handle)
            {
                if (dispatcher)
                {
                    dispatcher->post([handle]
                        {
                            handle.resume();
                        });
                }
                else
                {
                    handle.resume();
                }
            }
        }

        struct PromiseContext
        {
            std::shared_ptr<IDispatcher> _dispatcher;
            std::coroutine_handle<> _awaitingCoroutine;
            std::shared_ptr<IDispatcher> _awaitingDispatcher;
            bool _started = false;

            void setDispatcher(std::shared_ptr<IDispatcher> dispatcher)
            {
                _dispatcher = std::move(dispatcher);
            }

            void setDispatcherIfEmpty(const std::shared_ptr<IDispatcher>& dispatcher)
            {
                if (!_dispatcher)
                {
                    _dispatcher = dispatcher;
                }
            }

            void resumeAwaiting()
            {
                std::coroutine_handle<> awaiting_coroutine = std::exchange(_awaitingCoroutine, nullptr);
                std::shared_ptr<IDispatcher> awaiting_dispatcher = std::exchange(_awaitingDispatcher, nullptr);

                resume(std::move(awaiting_dispatcher), awaiting_coroutine);
            }
        };

        template <class Promise>
        void start(std::coroutine_handle<Promise> handle)
        {
            Promise& promise = handle.promise();

            if (!promise._started && !handle.done())
            {
                promise._started = true;
                resume(promise._dispatcher, handle);
            }
        }

        template<typename T>
        class TaskPromise : public PromiseContext
        {
        public:

            // value to be computed
            // when Task is not completed (coroutine didn't co_return anything yet) value is empty
            std::optional<T> _value;
            std::exception_ptr _exception;

            void rethrow() const
            {
                if (_exception)
                {
                    std::rethrow_exception(_exception);
                }
            }

            // Task is async result of our coroutine.
            Task<T> get_return_object();

            std::suspend_always initial_suspend() noexcept
            {
                return {};
            }

            // store value to be returned to awaiting coroutine or accessed through 'get' function
            void return_value(T val) noexcept
            {
                rethrow();

                _value = std::move(val);
            }

            void unhandled_exception() noexcept
            {
                // alternatively we can store current exception in std::exception_ptr to rethrow it later
                _exception = std::current_exception();
            }

            auto final_suspend() noexcept
            {
                struct transfer_awaitable
                {
                    bool await_ready() noexcept
                    {
                        return false;
                    }

                    void await_suspend(std::coroutine_handle<TaskPromise> handle) noexcept
                    {
                        handle.promise().resumeAwaiting();
                    }

                    void await_resume() noexcept {}
                };

                return transfer_awaitable{};
            }

            template<typename U>
            auto await_transform(Task<U>& task) noexcept;

            template<typename U>
            auto await_transform(Task<U>&& task) noexcept;

            template<class Awaitable>
            Awaitable& await_transform(Awaitable& awaitable) noexcept
            {
                return awaitable;
            }

            template<class Awaitable>
            Awaitable&& await_transform(Awaitable&& awaitable) noexcept
            {
                return std::move(awaitable);
            }
        };

        template<>
        class TaskPromise<void> : public PromiseContext
        {
        public:

            std::exception_ptr _exception;

            void rethrow() const
            {
                if (_exception)
                {
                    std::rethrow_exception(_exception);
                }
            }

            // Task is async result of our coroutine.
            Task<void> get_return_object();

            std::suspend_always initial_suspend() noexcept
            {
                return {};
            }

            // store value to be returned to awaiting coroutine or accessed through 'get' function
            void return_void()
            {
                rethrow();
            }

            void unhandled_exception()
            {
                // alternatively we can store current exception in std::exception_ptr to rethrow it later
                _exception = std::current_exception();
            }

            auto final_suspend() noexcept
            {
                struct transfer_awaitable
                {
                    bool await_ready() noexcept
                    {
                        return false;
                    }

                    void await_suspend(std::coroutine_handle<TaskPromise> handle) noexcept
                    {
                        handle.promise().resumeAwaiting();
                    }

                    void await_resume() noexcept {}
                };

                return transfer_awaitable{};
            }

            template<typename U>
            auto await_transform(Task<U>& task) noexcept;

            template<typename U>
            auto await_transform(Task<U>&& task) noexcept;

            template<class Awaitable>
            Awaitable& await_transform(Awaitable& awaitable) noexcept
            {
                return awaitable;
            }

            template<class Awaitable>
            Awaitable&& await_transform(Awaitable&& awaitable) noexcept
            {
                return std::move(awaitable);
            }
        };
    }

    template<typename T>
    struct Task
    {
        // declare promise type
        using promise_type = detail::TaskPromise<T>;

        Task() : _h(nullptr) {}

        Task(std::coroutine_handle<promise_type> handle) noexcept : _h(handle) {}

        Task(Task&& other) noexcept : _h(std::exchange(other._h, nullptr)) {}

        Task& operator=(Task&& other) noexcept
        {
            free();

            _h = std::exchange(other._h, nullptr);

            return *this;
        }

        ~Task()
        {
            free();
        }

        // interface for extracting value without awaiting on it
        bool is_ready() const
        {
            check_handle();

            _h.promise().rethrow();

            if constexpr (std::is_same_v<T, void>)
            {
                return _h.done();
            }
            else
            {
                return _h.promise()._value.has_value();
            }
        }

        T get()
        {
            check_handle();

            if (!is_ready())
            {
                std::terminate();
            }

            _h.promise().rethrow();

            if constexpr (!std::is_same_v<T, void>)
            {
                return std::move(*_h.promise()._value);
            }
        }

        void check_handle() const
        {
            if (!_h)
            {
                //Uninitialized Task without promise
                std::terminate();
            }
        }

        void free()
        {
            if (_h)
            {
                _h.destroy();

                _h = nullptr;
            }
        }

        auto await(std::shared_ptr<IDispatcher> awaiting_dispatcher) & noexcept;

        auto await(std::shared_ptr<IDispatcher> awaiting_dispatcher) && noexcept;

        std::coroutine_handle<promise_type> _h;
    };

    namespace detail
    {
        template<typename T>
        class TaskAwaiter
        {
        public:

            TaskAwaiter(Task<T>& task, std::shared_ptr<IDispatcher> awaiting_dispatcher) noexcept :
                _h(task._h),
                _awaitingDispatcher(std::move(awaiting_dispatcher))
            {}

            TaskAwaiter(Task<T>&& task, std::shared_ptr<IDispatcher> awaiting_dispatcher) noexcept :
                _task(std::move(task)),
                _h(_task->_h),
                _awaitingDispatcher(std::move(awaiting_dispatcher))
            {}

            bool await_ready()
            {
                return _h.done();
            }

            void await_suspend(std::coroutine_handle<> h)
            {
                typename Task<T>::promise_type& promise = _h.promise();

                if (promise._awaitingCoroutine)
                {
                    std::terminate();
                }

                promise._awaitingCoroutine = h;
                promise._awaitingDispatcher = _awaitingDispatcher;

                start(_h);
            }

            auto await_resume()
            {
                typename Task<T>::promise_type& promise = _h.promise();

                promise.rethrow();

                if constexpr (!std::is_same_v<T, void>)
                {
                    if (!promise._value)
                    {
                        std::terminate();
                    }

                    return std::move(*promise._value);
                }
            }

        private:

            std::optional<Task<T>> _task;
            std::coroutine_handle<typename Task<T>::promise_type> _h;
            std::shared_ptr<IDispatcher> _awaitingDispatcher;
        };
    }

    template<typename T>
    Task<T> detail::TaskPromise<T>::get_return_object()
    {
        return { std::coroutine_handle<TaskPromise>::from_promise(*this) };
    }

    inline Task<void> detail::TaskPromise<void>::get_return_object()
    {
        return { std::coroutine_handle<TaskPromise>::from_promise(*this) };
    }

    template<typename T>
    auto Task<T>::await(std::shared_ptr<IDispatcher> awaiting_dispatcher) & noexcept
    {
        return detail::TaskAwaiter<T>(*this, std::move(awaiting_dispatcher));
    }

    template<typename T>
    auto Task<T>::await(std::shared_ptr<IDispatcher> awaiting_dispatcher) && noexcept
    {
        return detail::TaskAwaiter<T>(std::move(*this), std::move(awaiting_dispatcher));
    }

    template<typename T>
    Task<T> coSpawn(Task<T> task, std::shared_ptr<IDispatcher> dispatcher = nullptr)
    {
        task.check_handle();
        task._h.promise().setDispatcher(std::move(dispatcher));
        detail::start(task._h);

        return task;
    }

    // also we can await other Task<T>
    template<typename T>
    auto operator co_await(Task<T>& task) noexcept
    {
        return task.await(nullptr);
    }

    template<typename T>
    auto operator co_await(Task<T>&& task) noexcept
    {
        return std::move(task).await(nullptr);
    }

    namespace detail
    {
        template<typename T>
        template<typename U>
        auto TaskPromise<T>::await_transform(Task<U>& task) noexcept
        {
            task._h.promise().setDispatcherIfEmpty(_dispatcher);

            return task.await(_dispatcher);
        }

        template<typename T>
        template<typename U>
        auto TaskPromise<T>::await_transform(Task<U>&& task) noexcept
        {
            task._h.promise().setDispatcherIfEmpty(_dispatcher);

            return std::move(task).await(_dispatcher);
        }

        template<typename U>
        auto TaskPromise<void>::await_transform(Task<U>& task) noexcept
        {
            task._h.promise().setDispatcherIfEmpty(_dispatcher);

            return task.await(_dispatcher);
        }

        template<typename U>
        auto TaskPromise<void>::await_transform(Task<U>&& task) noexcept
        {
            task._h.promise().setDispatcherIfEmpty(_dispatcher);

            return std::move(task).await(_dispatcher);
        }
    }
}
