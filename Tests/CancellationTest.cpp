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
#include "Awl/Time.h"

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

        out(awl::format() << _T("Main thread ") << std::this_thread::get_id());

        std::exception_ptr ex_ptr = nullptr;

        std::jthread client([&context, &out, client_sleep_time](std::stop_token token)
        {
            out(awl::format() << _T("Client ") << std::this_thread::get_id() << _T(" started "));
                
            //Is not called because client thread is already finished.
            std::stop_callback stop_wait
            {
                token,
                [&out]()
                {
                    out(awl::format() << _T("Client stop callback on thread ") << std::this_thread::get_id());
                }
            };

            awl::StopWatch sw;

            awl::sleep_for(Duration(client_sleep_time), token);

            const auto elapsed = sw.GetElapsedCast<Duration>();

            out(awl::format() << _T("Client has woken up within ") << sw << _T(" and finished."));
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
                    out(awl::format() << _T("Worker ") << std::this_thread::get_id() << _T(" started "));

                    //Is not called because client thread is already finished.
                    std::stop_callback stop_wait
                    {
                        token,
                        [&out]()
                        {
                            out(awl::format() << _T("Worker stop callback on thread ") << std::this_thread::get_id());
                        }
                    };

                    awl::StopWatch sw;

                    awl::sleep_for(Duration(worker_sleep_time), token);

                    out(awl::format() << _T("Worker has woken up within ") << sw);

                    const auto elapsed = sw.GetElapsedCast<Duration>();

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

        client.join();

        client.request_stop();

        for (auto & t : v)
        {
            t.join();
        }

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

AWT_EXAMPLE(Cancellation_JThread)
{
    using namespace std::chrono_literals;

    // A sleepy worker thread
    std::jthread sleepy_worker([&context](std::stop_token stoken)
    {
        for (int i = 10; i; --i)
        {
            std::this_thread::sleep_for(300ms);

            if (stoken.stop_requested())
            {
                context.out << "Sleepy worker is requested to stop\n";
                return;
            }

            context.out << "Sleepy worker goes back to sleep\n";
        }
    });

    // A waiting worker thread
    // The condition variable will be awoken by the stop request.
    std::jthread waiting_worker([&context](std::stop_token stoken)
    {
        std::mutex mutex;
        std::unique_lock lock(mutex);

        std::condition_variable_any().wait(lock, stoken, [&stoken] { return false; });

        if (stoken.stop_requested())
        {
            context.out << "Waiting worker is requested to stop\n";
        }
    });

    // std::jthread::request_stop() can be called explicitly:
    context.out << "Requesting stop of sleepy worker\n";
    sleepy_worker.request_stop();
    sleepy_worker.join();
    context.out << "Sleepy worker joined\n";

    // Or automatically using RAII:
    // waiting_worker's destructor will call request_stop()
    // and join the thread automatically.
}
