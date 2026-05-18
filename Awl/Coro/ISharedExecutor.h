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
    class ISharedExecutor
    {
    public:

        virtual ~ISharedExecutor() = default;

        virtual Awaitable<void> execute(std::move_only_function<void()> func) = 0;
    };

    class ISharedDispatcher : public ISharedExecutor
    {
    public:

        virtual void post(std::move_only_function<void()> func) = 0;

        virtual void join() = 0;
    };

    namespace detail
    {
        template <class Func>
        using shared_executor_result_t = std::remove_cvref_t<std::invoke_result_t<Func&>>;
    }

    template <class Func>
    Task<detail::shared_executor_result_t<Func>> execute(
        ISharedExecutor& executor,
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
            using Value = detail::shared_executor_result_t<Func>;
            std::optional<Value> result;

            co_await executor.execute([func = std::move(func), &result]() mutable
                {
                    result.emplace(std::invoke(func));
                });

            co_return std::move(*result);
        }
    }

    template <class Func>
    Task<detail::shared_executor_result_t<Func>> execute(
        const std::shared_ptr<ISharedExecutor>& executor,
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
