/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Coro/Job.h"
#include "Awl/Coro/Task.h"
#include "Awl/Coro/AsyncGenerator.h"
#include "Awl/Testing/UnitTest.h"
#include "Helpers/TimeQueue.h"
#include "Awl/StringFormat.h"

//Why does it fail?
//static_assert(std::ranges::range<awl::coro::async_generator<int>>);

namespace
{
    using namespace std::chrono_literals;

    awl::coro::async_generator<int> gen(awl::testing::ITimeQueue& time_queue, int count)
    {
        for (int i = 0; i < count; ++i)
        {
            // std::generator has deleted await_transform()
            co_await awl::testing::TimeQueueAwaitable(time_queue, 100ms);

            if (i > 5)
            {
                throw std::runtime_error("Generator overflow.");
            }

            co_yield i;
        }
    }

    awl::coro::Task<void> print(
        const awl::testing::TestContext& context,
        awl::testing::ITimeQueue& time_queue,
        int count,
        std::optional<int> limit = {})
    {
        //Unfortunately, 'for co_await' syntax is not approved for C++20 (I hope for now!) and instead of an elegant code we have to write
        //old school for loop with previously captured by rvalue generator.
        //for co_await(int i : gen())

        auto g = gen(time_queue, count);

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

    awl::coro::Job test(const awl::testing::TestContext& context, awl::testing::ITimeQueue& time_queue)
    {
        co_await print(context, time_queue, 3);

        co_await print(context, time_queue, 10, 2);

        try
        {
            co_await print(context, time_queue, 10);

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

    awl::coro::Job task = awl::coro::coSpawn(test(context, time_queue));

    AWL_ASSERT(!task.done());

    time_queue.loop(3);

    AWL_ASSERT(!task.done());

    time_queue.loop();

    AWL_ASSERT(task.done());
}
