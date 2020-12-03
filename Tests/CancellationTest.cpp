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

//It crashes on WSL with GCC. Tell me why? Run
//./AwlTest --filter Cancellation_InterruptibleSleep_Test --thread_count 100 --iteration_count 1000000
//and then Ctrl+Z and bg or fg and you get:
//terminate called without an active exception
//Aborted (core dumped)
//
//Previously it was:
//terminate called recursively
//terminate called recursively
AWT_TEST(Cancellation_InterruptibleSleep)
{
    AWT_ATTRIBUTE(int, client_sleep_time, default_client_sleep_time);
    AWT_ATTRIBUTE(int, worker_sleep_time, default_worker_sleep_time);
    AWT_ATTRIBUTE(size_t, thread_count, 10);
    AWT_ATTRIBUTE(size_t, iteration_count, 1);
    
    for (size_t iteration : awl::make_count(iteration_count))
    {
        std::mutex ex_mutex;
        std::exception_ptr ex_ptr = nullptr;

        static_cast<void>(iteration);

        awl::CancellationFlag cancellation;

        std::vector<std::thread> v;
        v.reserve(thread_count);

        for (size_t i = 0; i < thread_count; ++i)
        {
            v.push_back(std::thread([&context, &cancellation, client_sleep_time, worker_sleep_time, &ex_mutex, &ex_ptr]()
            {
                try
                {
                    awl::StopWatch w;

                    cancellation.Sleep(Duration(worker_sleep_time));

                    const auto elapsed = w.GetElapsedCast<Duration>();

                    AWT_ASSERT(elapsed.count() >= client_sleep_time);

                    AWT_ASSERT(elapsed.count() < worker_sleep_time);
                }
                catch (const std::exception &)
                {
                    std::lock_guard lock(ex_mutex);

                    if (ex_ptr == nullptr)
                    {
                        ex_ptr = std::current_exception();
                    }
                }
            }));
        }

        std::thread client([&context, &cancellation, client_sleep_time, &ex_mutex, &ex_ptr]()
        {
            try
            {
                std::this_thread::sleep_for(Duration(client_sleep_time));

                cancellation.Cancel();

                AWT_ASSERT(cancellation.IsCancelled());
            }
            catch (const std::exception &)
            {
                std::lock_guard lock(ex_mutex);

                if (ex_ptr == nullptr)
                {
                    ex_ptr = std::current_exception();
                }
            }
        });

        for (auto & t : v)
        {
            t.join();
        }

        client.join();

        //We store only the first exception.
        if (ex_ptr != nullptr)
        {
            std::rethrow_exception(ex_ptr);
        }
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
