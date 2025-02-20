/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Separator.h"
#include "Awl/Coro/UpdateTask.h"
#include "Awl/Coro/ProcessTask.h"
#include "Awl/Coro/Controller.h"
#include "Awl/Coro/AsyncGenerator.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/TimeQueue.h"

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
        awl::separator sep(_T(','));
        
        //Unfortunately, 'for co_await' syntax is not approved for C++20 (I hope for now!) and instead of an elegant code we have to write
        //old school for loop with previously captured by rvalue generator.
        //for co_await(int i : gen())

        auto g = gen(count);

        int n = 0;

        for (auto i = co_await g.begin(); i != g.end(); co_await ++i)
        {
            context.out << sep << *i;
            context.out.flush();

            if (limit && ++n == *limit)
            {
                break;
            }
        }

        context.out << std::endl;
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
            context.out << std::endl << "Exception: " << ex.what() << std::endl;
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
    awl::Controller controller;

    controller.register_task(test(context));

    awl::testing::timeQueue.loop(3);

    AWL_ASSERT_EQUAL(1u, controller.task_count());

    context.out << std::endl;

    // This invalidates timeQueue.
    controller.cancel();

    AWL_ASSERT_EQUAL(0u, controller.task_count());

    awl::testing::timeQueue.clear();
}

AWL_TEST(CoroControllerRegistered)
{
    awl::Controller controller;

    controller.register_task(test(context));

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

        context.out << id << " finished" << std::endl;
    }
}

namespace awl
{
    class ControllerTest
    {
    public:

        static awl::UpdateTask TestWaitAllTask(const awl::testing::TestContext& context, awl::Controller& controller)
        {
            RegisterTasks(context, controller);

            co_await controller.wait_all_task_experimental();
        }

        static awl::UpdateTask TestWait(const awl::testing::TestContext& context, awl::Controller& controller, bool all_task = false, std::size_t actual_N = 2)
        {
            RegisterTasks(context, controller);

            co_await controller.wait_any();

            context.out << "wait_any() finished" << std::endl;

            AWL_ASSERT_EQUAL(actual_N, controller.task_count());

            if (all_task)
            {
                co_await controller.wait_all_task_experimental();
            }
            else
            {
                co_await controller.wait_all();
            }

            context.out << "wait_all() finished" << std::endl;

            AWL_ASSERT_EQUAL(0u, controller.task_count());
        }

    private:

        static void RegisterTasks(const awl::testing::TestContext& context, awl::Controller& controller)
        {
            controller.register_task(PrintFinished(context, 1));
            controller.register_task(PrintFinished(context, 2));
            controller.register_task(PrintFinished(context, 3));

            AWL_ASSERT_EQUAL(3u, controller.task_count());
        }
    };
}

AWL_TEST(CoroControllerWaitAllTask)
{
    awl::Controller controller;

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
    awl::Controller controller;

    awl::UpdateTask task = awl::ControllerTest::TestWait(context, controller);

    awl::testing::timeQueue.loop();

    AWL_ASSERT(task.done());
}

AWL_TEST(CoroControllerCancelWait1)
{
    AWL_FLAG(all_task);

    awl::Controller controller;

    awl::UpdateTask task = awl::ControllerTest::TestWait(context, controller, all_task, 0);

    controller.cancel();

    AWL_ASSERT(task.done());

    awl::testing::timeQueue.clear();
}

// Fails with all_task=true
AWL_TEST(CoroControllerCancelWait2)
{
    AWL_FLAG(all_task);

    awl::Controller controller;

    awl::UpdateTask task = awl::ControllerTest::TestWait(context, controller, all_task);

    awl::testing::timeQueue.loop(1);

    controller.cancel();

    AWL_ASSERT(task.done());

    awl::testing::timeQueue.clear();
}
