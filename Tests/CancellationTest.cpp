#include <thread>
#include <chrono>
#include <vector>

#include "Awl/Cancellation.h"
#include "Awl/StopWatch.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/IntRange.h"

AWT_TEST(Cancellation_NegativeTimeDiff)
{
    AWT_UNUSED_CONTEXT;
    
    auto start = std::chrono::steady_clock::now();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto stop = std::chrono::steady_clock::now();

    AWT_ASSERT(stop > start);

    auto diff = start - stop;

    AWT_ASSERT(diff < std::chrono::milliseconds(0));
    AWT_ASSERT(diff.count() < 0);
}

static constexpr int default_client_sleep_time = 100;
static constexpr int default_worker_sleep_time = 1000;

using Duration = std::chrono::milliseconds;

//It crashes on Linux with GCC!
AWT_DISABLED_TEST(Cancellation_InterruptibleSleep)
{
    AWT_ATTRIBUTE(int, client_sleep_time, default_client_sleep_time);
    AWT_ATTRIBUTE(int, worker_sleep_time, default_worker_sleep_time);
    AWT_ATTRIBUTE(size_t, thread_count, 10);
    AWT_ATTRIBUTE(size_t, iteration_count, 10);
    
    for (size_t iteration : awl::make_count(iteration_count))
    {
        static_cast<void>(iteration);

        awl::CancellationFlag cancellation;

        std::vector<std::thread> v;
        v.reserve(thread_count);

        for (size_t i = 0; i < thread_count; ++i)
        {
            v.push_back(std::thread([&context, &cancellation, client_sleep_time, worker_sleep_time]()
            {
                awl::StopWatch w;

                cancellation.Sleep(Duration(worker_sleep_time));

                const auto elapsed = w.GetElapsedCast<Duration>();

                AWT_ASSERT(elapsed.count() >= client_sleep_time);

                AWT_ASSERT(elapsed.count() < worker_sleep_time);
            }));
        }

        std::thread client([&context, &cancellation, client_sleep_time]()
        {
            std::this_thread::sleep_for(Duration(client_sleep_time));

            cancellation.Cancel();

            AWT_ASSERT(cancellation.IsCancelled());
        });

        for (auto & t : v)
        {
            t.join();
        }

        client.join();
    }
}

AWT_TEST(Cancellation_SimpleSleep)
{
    AWT_ATTRIBUTE(int, client_sleep_time, default_client_sleep_time);

    awl::CancellationFlag cancellation;

    awl::StopWatch w;

    cancellation.Sleep(Duration(client_sleep_time));

    const auto elapsed = w.GetElapsedCast<Duration>();

    AWT_ASSERT(elapsed.count() >= client_sleep_time);
}
