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

    AWL_ASSERT_EQUAL(1, controller.task_count());

    context.out << std::endl;

    // This invalidates timeQueue.
    controller.cancel();

    AWL_ASSERT_EQUAL(0, controller.task_count());

    awl::testing::timeQueue.clear();
}

AWL_TEST(CoroControllerRegistered)
{
    awl::Controller controller;

    controller.register_task(test(context));

    AWL_ASSERT_EQUAL(1, controller.task_count());

    awl::testing::timeQueue.loop(3);

    // The task is still in the list.
    AWL_ASSERT_EQUAL(1, controller.task_count());

    awl::testing::timeQueue.loop();

    // The task has removed itself automatically from the list.
    AWL_ASSERT_EQUAL(0, controller.task_count());
}

namespace
{
    awl::UpdateTask PrintFinished(const awl::testing::TestContext& context, int id)
    {
        co_await 100ms;

        context.out << "finished " << id << std::endl;
    }
}

AWL_TEST(CoroControllerWaitAll)
{
    awl::Controller controller;

    controller.register_task(PrintFinished(context, 1));
    controller.register_task(PrintFinished(context, 2));
    controller.register_task(PrintFinished(context, 3));

    AWL_ASSERT_EQUAL(3, controller.task_count());

    awl::UpdateTask task = controller.wait_all();

    awl::testing::timeQueue.loop(1);

    AWL_ASSERT_EQUAL(2, controller.task_count());

    awl::testing::timeQueue.loop(1);

    AWL_ASSERT_EQUAL(1, controller.task_count());

    awl::testing::timeQueue.loop(1);

    AWL_ASSERT_EQUAL(0, controller.task_count());

    AWL_ASSERT(awl::testing::timeQueue.empty());

    AWL_ASSERT(task.done());
}
