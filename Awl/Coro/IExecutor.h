#pragma once

#include "Awl/Coro/Awaitable.h"
#include "Awl/Coro/Task.h"

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace awl::coro
{
    class IExecutor
    {
    public:

        virtual ~IExecutor() = default;

        virtual Awaitable<void> execute(std::move_only_function<void()> func) = 0;
    };

    class IDispatcher : public IExecutor
    {
    public:

        virtual void post(std::move_only_function<void()> func) = 0;

        virtual void join() = 0;
    };

    namespace detail
    {
        template <class Func>
        using executor_result_t = std::remove_cvref_t<std::invoke_result_t<Func&>>;
    }

    template <class Func>
    Task<detail::executor_result_t<Func>> execute(
        IExecutor& executor,
        Func func)
    {
        using Result = std::invoke_result_t<Func&>;

        if constexpr (std::is_void_v<Result>)
        {
            co_await executor.execute([func = std::move(func)] mutable
                {
                    std::invoke(func);
                });

            co_return;
        }
        else
        {
            using Value = detail::executor_result_t<Func>;
            std::optional<Value> result;

            co_await executor.execute([func = std::move(func), &result]() mutable
                {
                    result.emplace(std::invoke(func));
                });

            co_return std::move(*result);
        }
    }

    template <class Func>
    Task<detail::executor_result_t<Func>> execute(
        const std::shared_ptr<IExecutor>& executor,
        Func func)
    {
        if constexpr (std::is_void_v<std::invoke_result_t<Func&>>)
        {
            co_await execute(*executor, std::move(func));

            co_return;
        }
        else
        {
            co_return co_await execute(*executor, std::move(func));
        }
    }
}
