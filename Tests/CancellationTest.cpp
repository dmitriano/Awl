#include <thread>
#include <chrono>
#include <vector>

#include "Awl/Cancellation.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

AWL_TEST(Cancellation_NegativeTimeDiff)
{
    auto start = std::chrono::steady_clock::now();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto stop = std::chrono::steady_clock::now();

    Assert::IsTrue(stop > start);

    auto diff = start - stop;

    Assert::IsTrue(diff < std::chrono::milliseconds(0));
    Assert::IsTrue(diff.count() < 0);
}

constexpr int client_sleep_time = 100;
constexpr int worker_sleep_time = 1000;

typedef std::chrono::milliseconds Duration;

AWL_TEST(Cancellation_InterruptibleSleep)
{
    awl::Cancellation cancellation;

    constexpr size_t thread_count = 10;

    std::vector<std::thread> v;
    v.reserve(thread_count);

    for (size_t i = 0; i < thread_count; ++i)
    {
        v.push_back(std::thread([&cancellation]()
        {
            auto start = std::chrono::steady_clock::now();

            cancellation.Sleep(Duration(worker_sleep_time));

            auto now = std::chrono::steady_clock::now();

            auto ellapsed = std::chrono::duration_cast<Duration>(now - start);

            Assert::IsTrue(ellapsed.count() >= client_sleep_time);

            Assert::IsTrue(ellapsed.count() < worker_sleep_time);
        }));
    }

    std::thread client([&cancellation]()
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

AWL_TEST(Cancellation_SimpleSleep)
{
    awl::Cancellation cancellation;

    auto start = std::chrono::steady_clock::now();

    cancellation.Sleep(Duration(client_sleep_time));

    auto now = std::chrono::steady_clock::now();

    auto ellapsed = std::chrono::duration_cast<Duration>(now - start);

    Assert::IsTrue(ellapsed.count() >= client_sleep_time);
}
