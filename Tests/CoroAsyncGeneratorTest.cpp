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
        //There is no 'for co_await' yet in C++20.
        //for co_await(int i : gen())

        awl::separator sep(_T(','));
        
        auto g = gen();

        auto i = co_await g.begin();

        while (i != g.end())
        {
            context.out << sep << *i;

            co_await ++i;
        }

        context.out << std::endl;
    }
}

// main can't be a coroutine and usually need some sort of looper (io_service or timer loop in this example)
AWT_EXAMPLE(CoroAsyncGenerator)
{
    awl::UpdateTask task = print(context);

    // execute deferred coroutines
    time_queue.loop();
}
