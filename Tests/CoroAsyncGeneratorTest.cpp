/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Coro/Job.h"
#include "Awl/Coro/Task.h"
#include "Awl/Coro/TaskPool.h"
#include "Awl/Coro/AsyncGenerator.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/TimeQueue.h"
#include "Awl/StringFormat.h"

//Why does it fail?
//static_assert(std::ranges::range<awl::async_generator<int>>);

namespace
{
    using namespace std::chrono_literals;

    awl::async_generator<int> gen(awl::coro::IDelayedExecutor& delayed_executor, int count)
    {
        for (int i = 0; i < count; ++i)
        {
            // std::generator has deleted await_transform()
            co_await awl::coro::DelayedAwaitable(delayed_executor, 100ms);

            if (i > 5)
            {
                throw std::runtime_error("Generator overflow.");
            }

            co_yield i;
        }
    }

    awl::Task<void> print(
        const awl::testing::TestContext& context,
        awl::coro::IDelayedExecutor& delayed_executor,
        int count,
        std::optional<int> limit = {})
    {
        //Unfortunately, 'for co_await' syntax is not approved for C++20 (I hope for now!) and instead of an elegant code we have to write
        //old school for loop with previously captured by rvalue generator.
        //for co_await(int i : gen())

        auto g = gen(delayed_executor, count);

        int n = 0;

        awl::ostringstream line;
        bool first = true;

        for (auto i = co_await g.begin(); i != g.end(); co_await ++i)
        {
            if (!first)
            {
                line << _T(", ");
            }
            else
            {
                first = false;
            }

            line << *i;

            if (limit && ++n == *limit)
            {
                break;
            }
        }

        context.logger->debug(line.str());
    }

    awl::Job test(const awl::testing::TestContext& context, awl::coro::IDelayedExecutor& delayed_executor)
    {
        co_await print(context, delayed_executor, 3);

        co_await print(context, delayed_executor, 10, 2);

        try
        {
            co_await print(context, delayed_executor, 10);

            AWL_FAILM(_T("AsyncGenerator did not throw."));
        }
        catch (const std::exception& ex)
        {
            context.logger->debug(_T("\nException: {}"), awl::fromACString(ex.what()));
        }
        catch (...)
        {
            AWL_FAILM(_T("AsyncGenerator thrown a wrong exception."));
        }
    }
}

AWL_TEST(CoroAsyncGeneratorOwned)
{
    awl::testing::TimeQueue time_queue;

    awl::Job task = test(context, time_queue);

    AWL_ASSERT(!task.done());

    time_queue.loop(3);

    AWL_ASSERT(!task.done());

    time_queue.loop();

    AWL_ASSERT(task.done());
}

AWL_TEST(CoroControllerCancel)
{
    awl::testing::TimeQueue time_queue;
    awl::TaskPool controller;

    controller.spawn(test(context, time_queue));

    time_queue.loop(3);

    AWL_ASSERT_EQUAL(1u, controller.task_count());

    context.logger->debug(_T(""));

    // This invalidates time_queue.
    controller.cancel();

    AWL_ASSERT_EQUAL(0u, controller.task_count());

    time_queue.clear();
}

AWL_TEST(CoroControllerRegistered)
{
    awl::testing::TimeQueue time_queue;
    awl::TaskPool controller;

    controller.spawn(test(context, time_queue));

    AWL_ASSERT_EQUAL(1u, controller.task_count());

    time_queue.loop(3);

    // The task is still in the list.
    AWL_ASSERT_EQUAL(1u, controller.task_count());

    time_queue.loop();

    // The task has removed itself automatically from the list.
    AWL_ASSERT_EQUAL(0u, controller.task_count());
}

namespace
{
    awl::Job PrintFinished(
        const awl::testing::TestContext& context,
        awl::coro::IDelayedExecutor& delayed_executor,
        int id)
    {
        co_await awl::coro::DelayedAwaitable(delayed_executor, 100ms);

        context.logger->debug(_T("{} finished"), id);
    }
}

namespace awl
{
    class ControllerTest
    {
    public:

        static awl::Job TestWaitAllTask(
            const awl::testing::TestContext& context,
            awl::coro::IDelayedExecutor& delayed_executor,
            awl::TaskPool& controller)
        {
            RegisterTasks(context, delayed_executor, controller);

            co_await controller.wait_all_task_experimental();
        }

        static awl::Job TestWait(
            const awl::testing::TestContext& context,
            awl::coro::IDelayedExecutor& delayed_executor,
            awl::TaskPool& controller,
            bool all_task = false,
            std::size_t actual_N = 2)
        {
            RegisterTasks(context, delayed_executor, controller);

            context.logger->debug("wait_any() started");

            co_await controller.wait_any();

            context.logger->debug("wait_any() finished");

            AWL_ASSERT_EQUAL(actual_N, controller.task_count());

            if (all_task)
            {
                co_await controller.wait_all_task_experimental();
            }
            else
            {
                co_await controller.wait_all();
            }

            context.logger->debug("wait_all() finished");

            AWL_ASSERT_EQUAL(0u, controller.task_count());
        }

    private:

        static void RegisterTasks(
            const awl::testing::TestContext& context,
            awl::coro::IDelayedExecutor& delayed_executor,
            awl::TaskPool& controller)
        {
            controller.spawn(PrintFinished(context, delayed_executor, 1));
            controller.spawn(PrintFinished(context, delayed_executor, 2));
            controller.spawn(PrintFinished(context, delayed_executor, 3));

            AWL_ASSERT_EQUAL(3u, controller.task_count());
        }
    };
}

AWL_TEST(CoroControllerWaitAllTask)
{
    awl::testing::TimeQueue time_queue;
    awl::TaskPool controller;

    awl::Job task = awl::ControllerTest::TestWaitAllTask(context, time_queue, controller);

    time_queue.loop(1);

    AWL_ASSERT_EQUAL(2u, controller.task_count());

    time_queue.loop(1);

    AWL_ASSERT_EQUAL(1u, controller.task_count());

    time_queue.loop(1);

    AWL_ASSERT_EQUAL(0u, controller.task_count());

    AWL_ASSERT(time_queue.empty());

    AWL_ASSERT(task.done());
}

AWL_TEST(CoroControllerWait)
{
    awl::testing::TimeQueue time_queue;
    awl::TaskPool controller;

    awl::Job task = awl::ControllerTest::TestWait(context, time_queue, controller);

    time_queue.loop();

    AWL_ASSERT(task.done());
}

AWL_TEST(CoroControllerCancelWait1)
{
    AWL_FLAG(all_task);

    awl::testing::TimeQueue time_queue;
    awl::TaskPool controller;

    awl::Job task = awl::ControllerTest::TestWait(context, time_queue, controller, all_task, 0);

    controller.cancel();

    AWL_ASSERT(task.done());

    time_queue.clear();
}

// Fails with all_task=true
// UB: Awl/Awl/Coro/TaskPool.cpp:62:5: runtime error: member access within address 0x616577be4d90 which does not point to an object of type 'Handler'
AWL_TEST(CoroControllerCancelWait2)
{
    AWL_FLAG(all_task);

    awl::testing::TimeQueue time_queue;
    awl::TaskPool controller;

    awl::Job task = awl::ControllerTest::TestWait(context, time_queue, controller, all_task);

    time_queue.loop(1);

    controller.cancel();

    AWL_ASSERT(task.done());

    time_queue.clear();
}
