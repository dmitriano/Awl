#include <thread>
#include <chrono>
#include <vector>

#include "Awl/Cancellation.h"
#include "Awl/StopWatch.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

AWT_TEST(Cancellation_NegativeTimeDiff)
{
    AWT_UNUSED_CONTEXT;

    auto start = std::chrono::steady_clock::now();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto stop = std::chrono::steady_clock::now();

    Assert::IsTrue(stop > start);

    auto diff = start - stop;

    Assert::IsTrue(diff < std::chrono::milliseconds(0));
    Assert::IsTrue(diff.count() < 0);
}

static constexpr int default_client_sleep_time = 100;
static constexpr int default_worker_sleep_time = 1000;

using Duration = std::chrono::milliseconds;

AWT_TEST(Cancellation_InterruptibleSleep)
{
    awl::CancellationFlag cancellation;

    AWL_ATTRIBUTE(int, client_sleep_time, default_client_sleep_time);
    AWL_ATTRIBUTE(int, worker_sleep_time, default_worker_sleep_time);
    AWL_ATTRIBUTE(size_t, thread_count, 10);
    
    std::vector<std::thread> v;
    v.reserve(thread_count);

    for (size_t i = 0; i < thread_count; ++i)
    {
        v.push_back(std::thread([&cancellation, client_sleep_time, worker_sleep_time]()
        {
            awl::StopWatch w;

            cancellation.Sleep(Duration(worker_sleep_time));

            const auto elapsed = w.GetElapsedCast<Duration>();

            Assert::IsTrue(elapsed.count() >= client_sleep_time);

            Assert::IsTrue(elapsed.count() < worker_sleep_time);
        }));
    }

    std::thread client([&cancellation, client_sleep_time]()
    {
        std::this_thread::sleep_for(Duration(client_sleep_time));

        cancellation.Cancel();

        Assert::IsTrue(cancellation.IsCancelled());
    });

    for (auto & t : v)
    {
        t.join();
    }

    client.join();
}

AWT_TEST(Cancellation_SimpleSleep)
{
    AWL_ATTRIBUTE(int, client_sleep_time, default_client_sleep_time);

    awl::CancellationFlag cancellation;

    awl::StopWatch w;

    cancellation.Sleep(Duration(client_sleep_time));

    const auto elapsed = w.GetElapsedCast<Duration>();

    Assert::IsTrue(elapsed.count() >= client_sleep_time);
}
