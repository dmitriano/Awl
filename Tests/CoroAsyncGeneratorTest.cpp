/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Separator.h"
#include "Awl/Coro/UpdateTask.h"
#include "Awl/Coro/AsyncGenerator.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/TimeQueue.h"

namespace
{
    awl::testing::TimeQueue time_queue;

    using namespace std::chrono_literals;

    awl::async_generator<int> gen()
    {
        for (int i = 0; i < 10; ++i)
        {
            co_await awl::testing::TimeAwaitable(time_queue, 100ms);

            co_yield i;
        }
    }

    awl::UpdateTask print(const awl::testing::TestContext& context)
    {
        awl::separator sep(_T(','));
        
        //Unfortunately, 'for co_await' syntax is not approved for C++20 (I hope for now!) and instead of an elegant code we have to write
        //old school for loop with previously captured by rvalue generator.
        //for co_await(int i : gen())

        auto g = gen();

        for (auto i = co_await g.begin(); i != g.end(); co_await ++i)
        {
            context.out << sep << *i;
            context.out.flush();
        }

        context.out << std::endl;
    }
}

AWT_EXAMPLE(CoroAsyncGenerator)
{
    awl::UpdateTask task = print(context);

    time_queue.loop();
}
