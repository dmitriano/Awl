/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Separator.h"
#include "Awl/Coro/UpdateTask.h"
#include "Awl/Coro/ProcessTask.h"
#include "Awl/Coro/AsyncGenerator.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/TimeQueue.h"

namespace
{
    awl::testing::TimeQueue time_queue;

    using namespace std::chrono_literals;

    awl::async_generator<int> gen(int count)
    {
        for (int i = 0; i < count; ++i)
        {
            co_await awl::testing::TimeAwaitable(time_queue, 100ms);

            if (i > 5)
            {
                throw std::runtime_error("Generator overflow.");
            }

            co_yield i;
        }
    }

    awl::ProcessTask<void> print(const awl::testing::TestContext& context, int count)
    {
        awl::separator sep(_T(','));
        
        //Unfortunately, 'for co_await' syntax is not approved for C++20 (I hope for now!) and instead of an elegant code we have to write
        //old school for loop with previously captured by rvalue generator.
        //for co_await(int i : gen())

        auto g = gen(count);

        for (auto i = co_await g.begin(); i != g.end(); co_await ++i)
        {
            context.out << sep << *i;
            context.out.flush();
        }

        context.out << std::endl;
    }

    awl::UpdateTask test(const awl::testing::TestContext& context)
    {
        co_await print(context, 3);

        try
        {
            co_await print(context, 10);

            AWT_FAILM(_T("AsyncGenerator did not throw."));
        }
        catch (const std::exception& ex)
        {
            context.out << std::endl << "Exception: " << ex.what() << std::endl;
        }
        catch (...)
        {
            AWT_FAILM(_T("AsyncGenerator thrown a wrong exception."));
        }
    }
}

AWT_TEST(CoroAsyncGenerator)
{
    awl::UpdateTask task = test(context);

    time_queue.loop();
}
