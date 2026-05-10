/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Coro/Task.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/TimeQueue.h"
#include "Awl/StringFormat.h"
namespace
{
    using namespace std::chrono_literals;

    awl::Task<int> wait_n(const awl::testing::TestContext& context, awl::coro::IDelayedExecutor& delayed_executor, int n)
    {
        context.logger->debug(_T("before wait {}\n"), n);
        co_await awl::coro::DelayedAwaitable(delayed_executor, std::chrono::seconds(n));
        context.logger->debug(_T("after wait {}\n"), n);
        co_return n;
    }

    awl::Task<int> test(const awl::testing::TestContext& context, awl::coro::IDelayedExecutor& delayed_executor)
    {
        for (auto c : "hello world\n")
        {
            context.logger->debug(_T("{}"), c);
            co_await awl::coro::DelayedAwaitable(delayed_executor, 100ms);
        }

        context.logger->debug("test step 1\n");
        auto w3 = wait_n(context, delayed_executor, 3);
        context.logger->debug("test step 2\n");
        auto w2 = wait_n(context, delayed_executor, 2);
        context.logger->debug("test step 3\n");
        auto w1 = wait_n(context, delayed_executor, 1);
        context.logger->debug("test step 4\n");
        auto r = co_await w2 + co_await w3;
        context.logger->debug("awaiting already computed coroutine\n");
        co_return co_await w1 + r;
    }

    awl::Task<int> wait_0(const awl::testing::TestContext& context, awl::coro::IDelayedExecutor& delayed_executor)
    {
        co_return co_await wait_n(context, delayed_executor, 0);
    }
}

// main can't be a coroutine and usually need some sort of looper (io_service or timer loop in this example)
AWL_EXAMPLE(CoroTimer)
{
    awl::testing::TimeQueue time_queue;

    auto result = test(context, time_queue);

    // execute deferred coroutines
    time_queue.loop();

    context.logger->debug(_T("result: {}"), result.get());
}

AWL_EXAMPLE(CoroTimer0)
{
    awl::testing::TimeQueue time_queue;

    auto result = wait_0(context, time_queue);

    // execute deferred coroutines
    time_queue.loop();

    context.logger->debug(_T("result: {}"), result.get());
}
