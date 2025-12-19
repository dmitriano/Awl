/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Coro/UpdateTask.h"
#include "Awl/Coro/ProcessTask.h"
#include "Awl/Coro/TaskPool.h"
#include "Awl/Coro/AsyncGenerator.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/TimeQueue.h"
#include "Awl/StringFormat.h"

//Why does it fail?
//static_assert(std::ranges::range<awl::async_generator<int>>);

namespace
{
    using awl::testing::operator co_await;
    using namespace std::chrono_literals;

    awl::async_generator<int> gen(int count)
    {
        for (int i = 0; i < count; ++i)
        {
            // std::generator has deleted await_transform()
            co_await 100ms;

            if (i > 5)
            {
                throw std::runtime_error("Generator overflow.");
            }

            co_yield i;
        }
    }

    awl::ProcessTask<void> print(const awl::testing::TestContext& context, int count, std::optional<int> limit = {})
    {
        //Unfortunately, 'for co_await' syntax is not approved for C++20 (I hope for now!) and instead of an elegant code we have to write
        //old school for loop with previously captured by rvalue generator.
        //for co_await(int i : gen())

        auto g = gen(count);

        int n = 0;

        awl::format line;
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

        context.logger.debug(line);
    }

    awl::UpdateTask test(const awl::testing::TestContext& context)
    {
        co_await print(context, 3);

        co_await print(context, 10, 2);

        try
        {
            co_await print(context, 10);

            AWL_FAILM(_T("AsyncGenerator did not throw."));
        }
        catch (const std::exception& ex)
        {
            context.logger.debug(awl::format() << awl::format::endl << "Exception: " << ex.what());
        }
        catch (...)
        {
            AWL_FAILM(_T("AsyncGenerator thrown a wrong exception."));
        }
    }
}

AWL_TEST(CoroAsyncGeneratorOwned)
{
    awl::UpdateTask task = test(context);

    AWL_ASSERT(!task.done());

    awl::testing::timeQueue.loop(3);

    AWL_ASSERT(!task.done());

    awl::testing::timeQueue.loop();

    AWL_ASSERT(task.done());
}

AWL_TEST(CoroControllerCancel)
{
    awl::TaskPool controller;

    controller.spawn(test(context));

    awl::testing::timeQueue.loop(3);

    AWL_ASSERT_EQUAL(1u, controller.task_count());

    context.logger.debug(awl::format());

    // This invalidates timeQueue.
    controller.cancel();

    AWL_ASSERT_EQUAL(0u, controller.task_count());

    awl::testing::timeQueue.clear();
}

AWL_TEST(CoroControllerRegistered)
{
    awl::TaskPool controller;

    controller.spawn(test(context));

    AWL_ASSERT_EQUAL(1u, controller.task_count());

    awl::testing::timeQueue.loop(3);

    // The task is still in the list.
    AWL_ASSERT_EQUAL(1u, controller.task_count());

    awl::testing::timeQueue.loop();

    // The task has removed itself automatically from the list.
    AWL_ASSERT_EQUAL(0u, controller.task_count());
}

namespace
{
    awl::UpdateTask PrintFinished(const awl::testing::TestContext& context, int id)
    {
        co_await 100ms;

        context.logger.debug(awl::format() << id << " finished");
    }
}

namespace awl
{
    class ControllerTest
    {
    public:

        static awl::UpdateTask TestWaitAllTask(const awl::testing::TestContext& context, awl::TaskPool& controller)
        {
            RegisterTasks(context, controller);

            co_await controller.wait_all_task_experimental();
        }

        static awl::UpdateTask TestWait(const awl::testing::TestContext& context, awl::TaskPool& controller, bool all_task = false, std::size_t actual_N = 2)
        {
            RegisterTasks(context, controller);

            context.logger.debug("wait_any() started");

            co_await controller.wait_any();

            context.logger.debug("wait_any() finished");

            AWL_ASSERT_EQUAL(actual_N, controller.task_count());

            if (all_task)
            {
                co_await controller.wait_all_task_experimental();
            }
            else
            {
                co_await controller.wait_all();
            }

            context.logger.debug("wait_all() finished");

            AWL_ASSERT_EQUAL(0u, controller.task_count());
        }

    private:

        static void RegisterTasks(const awl::testing::TestContext& context, awl::TaskPool& controller)
        {
            controller.spawn(PrintFinished(context, 1));
            controller.spawn(PrintFinished(context, 2));
            controller.spawn(PrintFinished(context, 3));

            AWL_ASSERT_EQUAL(3u, controller.task_count());
        }
    };
}

AWL_TEST(CoroControllerWaitAllTask)
{
    awl::TaskPool controller;

    awl::UpdateTask task = awl::ControllerTest::TestWaitAllTask(context, controller);

    awl::testing::timeQueue.loop(1);

    AWL_ASSERT_EQUAL(2u, controller.task_count());

    awl::testing::timeQueue.loop(1);

    AWL_ASSERT_EQUAL(1u, controller.task_count());

    awl::testing::timeQueue.loop(1);

    AWL_ASSERT_EQUAL(0u, controller.task_count());

    AWL_ASSERT(awl::testing::timeQueue.empty());

    AWL_ASSERT(task.done());
}

AWL_TEST(CoroControllerWait)
{
    awl::TaskPool controller;

    awl::UpdateTask task = awl::ControllerTest::TestWait(context, controller);

    awl::testing::timeQueue.loop();

    AWL_ASSERT(task.done());
}

AWL_TEST(CoroControllerCancelWait1)
{
    AWL_FLAG(all_task);

    awl::TaskPool controller;

    awl::UpdateTask task = awl::ControllerTest::TestWait(context, controller, all_task, 0);

    controller.cancel();

    AWL_ASSERT(task.done());

    awl::testing::timeQueue.clear();
}

// Fails with all_task=true
// UB: Awl/Awl/Coro/TaskPool.cpp:62:5: runtime error: member access within address 0x616577be4d90 which does not point to an object of type 'Handler'
AWL_TEST(CoroControllerCancelWait2)
{
    AWL_FLAG(all_task);

    awl::TaskPool controller;

    awl::UpdateTask task = awl::ControllerTest::TestWait(context, controller, all_task);

    awl::testing::timeQueue.loop(1);

    controller.cancel();

    AWL_ASSERT(task.done());

    awl::testing::timeQueue.clear();
}
