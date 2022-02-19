/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <chrono>
#include <vector>

#include "Awl/Cancellation.h"
#include "Awl/StopWatch.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/IntRange.h"
#include "Awl/StringFormat.h"

AWT_EXAMPLE(Cancellation_NegativeTimeDiff)
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

//./AwlTest --filter Cancellation_InterruptibleSleep_Example --thread_count 100 --iteration_count 1000000
//double lock of a mutex
AWT_UNSTABLE_EXAMPLE(Cancellation_InterruptibleSleep)
{
    AWT_ATTRIBUTE(int, client_sleep_time, default_client_sleep_time);
    AWT_ATTRIBUTE(int, worker_sleep_time, default_worker_sleep_time);
    AWT_ATTRIBUTE(size_t, thread_count, 3);
    AWT_ATTRIBUTE(size_t, iteration_count, 1);
    
    for (size_t iteration : awl::make_count(iteration_count))
    {
        static_cast<void>(iteration);

        std::mutex m;

        auto out = [&context, &m](const awl::String& text)
        {
            std::lock_guard lock(m);

            context.out << text << std::endl;
        };

        std::exception_ptr ex_ptr = nullptr;

        std::jthread client([&context, &out, client_sleep_time](std::stop_token token)
        {
            out(_T("Client  started."));
                
            awl::sleep_for(Duration(client_sleep_time), token);

            out(_T("Client finished."));
        });

        const std::stop_token token = client.get_stop_token();

        std::vector<std::thread> v;
        v.reserve(thread_count);

        for (size_t i : awl::make_count(thread_count))
        {
            static_cast<void>(i);

            v.push_back(std::thread([&context, &out, &token, client_sleep_time, worker_sleep_time, &m, &ex_ptr]()
            {
                    
                try
                {
                    out(_T("Worker started."));

                    awl::StopWatch w;

                    awl::sleep_for(Duration(worker_sleep_time), token);

                    const auto elapsed = w.GetElapsedCast<Duration>();

                    out(awl::format() << _T("Worker has woken up within ") << elapsed << _T("ms"));

                    //It is not quite correct to check this without some further synchronization,
                    //so the test will periodically fail.
                    AWT_ASSERT(elapsed.count() >= client_sleep_time);

                    //Ctrl+Z on Linux causes this assertion to fail.
                    AWT_ASSERT(elapsed.count() < worker_sleep_time);

                    out(_T("Worker succeeded."));
                }
                catch (const std::exception &)
                {
                    out(_T("Worker failed."));

                    if (ex_ptr == nullptr)
                    {
                        ex_ptr = std::current_exception();
                    }
                }
            }));
        }

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

AWT_EXAMPLE(Cancellation_SimpleSleep)
{
    AWT_ATTRIBUTE(int, client_sleep_time, default_client_sleep_time);

    awl::StopWatch w;

    awl::sleep_for(Duration(client_sleep_time), context.stopToken);

    const auto elapsed = w.GetElapsedCast<Duration>();

    AWT_ASSERT(elapsed.count() >= client_sleep_time);
}
