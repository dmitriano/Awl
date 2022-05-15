/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Coro/ProcessTask.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/Testing/TimeQueue.h"

#include <coroutine>
#include <optional>
#include <iostream>
#include <chrono>

namespace
{
    awl::testing::TimeQueue time_queue;

    using namespace std::chrono_literals;

    awl::ProcessTask<int> wait_n(int n)
    {
        std::cout << "before wait " << n << '\n';
        co_await awl::testing::TimeAwaitable(time_queue, std::chrono::seconds(n));
        std::cout << "after wait " << n << '\n';
        co_return n;
    }

    awl::ProcessTask<int> test()
    {
        for (auto c : "hello world\n")
        {
            std::cout << c;
            co_await awl::testing::TimeAwaitable(time_queue, 100ms);
        }

        std::cout << "test step 1\n";
        auto w3 = wait_n(3);
        std::cout << "test step 2\n";
        auto w2 = wait_n(2);
        std::cout << "test step 3\n";
        auto w1 = wait_n(1);
        std::cout << "test step 4\n";
        auto r = co_await w2 + co_await w3;
        std::cout << "awaiting already computed coroutine\n";
        co_return co_await w1 + r;
    }
}

// main can't be a coroutine and usually need some sort of looper (io_service or timer loop in this example)
AWT_EXAMPLE(CoroTimer)
{
    auto result = test();

    // execute deferred coroutines
    time_queue.loop();

    context.out << "result: " << result.get() << std::endl;
}
