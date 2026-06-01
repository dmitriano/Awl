/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Coro/Job.h"
#include "Awl/Coro/Task.h"
#include "Awl/Testing/UnitTest.h"
#include "Helpers/TimeQueue.h"
#include "Awl/StringFormat.h"

namespace
{
    using namespace std::chrono_literals;

    awl::coro::Task<int> wait_n(const awl::testing::TestContext& context, awl::testing::ITimeQueue& time_queue, int n)
    {
        context.logger->debug(_T("before wait {}\n"), n);
        co_await awl::testing::TimeQueueAwaitable(time_queue, std::chrono::seconds(n));
        context.logger->debug(_T("after wait {}\n"), n);
        co_return n;
    }

    awl::coro::Task<int> test(const awl::testing::TestContext& context, awl::testing::ITimeQueue& time_queue)
    {
        for (auto c : "hello world\n")
        {
            context.logger->debug(_T("{}"), c);
            co_await awl::testing::TimeQueueAwaitable(time_queue, 100ms);
        }

        context.logger->debug("test step 1\n");
        auto w3 = awl::coro::spawn(wait_n(context, time_queue, 3));
        context.logger->debug("test step 2\n");
        auto w2 = awl::coro::spawn(wait_n(context, time_queue, 2));
        context.logger->debug("test step 3\n");
        auto w1 = awl::coro::spawn(wait_n(context, time_queue, 1));
        context.logger->debug("test step 4\n");
        auto r = co_await w2 + co_await w3;
        context.logger->debug("awaiting already computed coroutine\n");
        co_return co_await w1 + r;
    }

    awl::coro::Task<int> wait_0(const awl::testing::TestContext& context, awl::testing::ITimeQueue& time_queue)
    {
        co_return co_await wait_n(context, time_queue, 0);
    }

    awl::coro::Job setFlag(bool& flag)
    {
        flag = true;

        co_return;
    }

    awl::coro::Task<int> setTaskFlag(bool& flag)
    {
        flag = true;

        co_return 7;
    }

    awl::coro::Task<int> synchronousValue()
    {
        co_return 7;
    }

    awl::coro::Job awaitSynchronousTask(int& result)
    {
        result = co_await synchronousValue();
    }

    awl::coro::Job awaitSynchronousJob(bool& flag)
    {
        co_await setFlag(flag);
    }
}

AWL_TEST(CoroJobStartsOnSpawn)
{
    AWL_UNUSED_CONTEXT;

    bool flag = false;
    awl::coro::Job job = setFlag(flag);

    AWL_ASSERT(!flag);
    AWL_ASSERT(!job.done());

    job = awl::coro::spawn(std::move(job));

    AWL_ASSERT(flag);
    AWL_ASSERT(job.done());
}

AWL_TEST(CoroTaskStartsOnSpawn)
{
    AWL_UNUSED_CONTEXT;

    bool flag = false;
    awl::coro::Task<int> task = setTaskFlag(flag);

    AWL_ASSERT(!flag);
    AWL_ASSERT(!task.is_ready());

    task = awl::coro::spawn(std::move(task));

    AWL_ASSERT(flag);
    AWL_ASSERT(task.is_ready());
    AWL_ASSERT_EQUAL(7, task.get());
}

AWL_TEST(CoroTaskAwaitsSynchronousTask)
{
    AWL_UNUSED_CONTEXT;

    int result = 0;
    awl::coro::Job job = awl::coro::spawn(awaitSynchronousTask(result));

    AWL_ASSERT(job.done());
    AWL_ASSERT_EQUAL(7, result);
}

AWL_TEST(CoroJobAwaitsSynchronousJob)
{
    AWL_UNUSED_CONTEXT;

    bool flag = false;
    awl::coro::Job job = awl::coro::spawn(awaitSynchronousJob(flag));

    AWL_ASSERT(job.done());
    AWL_ASSERT(flag);
}

// main can't be a coroutine and usually need some sort of looper (io_service or timer loop in this example)
AWL_EXAMPLE(CoroTimer)
{
    awl::testing::TimeQueue time_queue;

    auto result = awl::coro::spawn(test(context, time_queue));

    // execute deferred coroutines
    time_queue.loop();

    context.logger->debug(_T("result: {}"), result.get());
}

AWL_EXAMPLE(CoroTimer0)
{
    awl::testing::TimeQueue time_queue;

    auto result = awl::coro::spawn(wait_0(context, time_queue));

    // execute deferred coroutines
    time_queue.loop();

    context.logger->debug(_T("result: {}"), result.get());
}
