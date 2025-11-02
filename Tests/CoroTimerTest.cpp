/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Coro/ProcessTask.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/TimeQueue.h"
#include "Awl/StringFormat.h"

namespace
{
    using awl::testing::operator co_await;
    using namespace std::chrono_literals;

    awl::ProcessTask<int> wait_n(const awl::testing::TestContext& context, int n)
    {
        context.logger.debug(awl::format() << "before wait " << n << awl::format::endl);
        co_await std::chrono::seconds(n);
        context.logger.debug(awl::format() << "after wait " << n << awl::format::endl);
        co_return n;
    }

    awl::ProcessTask<int> test(const awl::testing::TestContext& context)
    {
        for (auto c : "hello world\n")
        {
            context.logger.debug(awl::format() << c);
            co_await 100ms;
        }

        context.logger.debug("test step 1\n");
        auto w3 = wait_n(context, 3);
        context.logger.debug("test step 2\n");
        auto w2 = wait_n(context, 2);
        context.logger.debug("test step 3\n");
        auto w1 = wait_n(context, 1);
        context.logger.debug("test step 4\n");
        auto r = co_await w2 + co_await w3;
        context.logger.debug("awaiting already computed coroutine\n");
        co_return co_await w1 + r;
    }

    awl::ProcessTask<int> wait_0(const awl::testing::TestContext& context)
    {
        co_return co_await wait_n(context, 0);
    }
}

// main can't be a coroutine and usually need some sort of looper (io_service or timer loop in this example)
AWL_EXAMPLE(CoroTimer)
{
    auto result = test(context);

    // execute deferred coroutines
    awl::testing::timeQueue.loop();

    context.logger.debug(awl::format() << "result: " << result.get());
}

AWL_EXAMPLE(CoroTimer0)
{
    auto result = wait_0(context);

    // execute deferred coroutines
    awl::testing::timeQueue.loop();

    context.logger.debug(awl::format() << "result: " << result.get());
}
