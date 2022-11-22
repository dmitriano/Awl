/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

//./AwlTest --filter Cancellation_InterruptibleSleep.* --output failed --loop 100 --thread_count 10
AWT_UNSTABLE_EXAMPLE(Cancellation_InterruptibleSleep)
{
    AWT_ATTRIBUTE(int, client_sleep_time, default_client_sleep_time);
    AWT_ATTRIBUTE(int, worker_sleep_time, default_worker_sleep_time);
    AWT_ATTRIBUTE(size_t, thread_count, 3);
    
    std::mutex m;

    auto out = [&context, &m](const awl::String& text)
    {
        std::lock_guard lock(m);

        context.out << text << std::endl;
    };

    out(awl::format() << _T("Main thread ") << std::this_thread::get_id());

    std::exception_ptr ex_ptr = nullptr;

    std::condition_variable event_cv;
    std::mutex event_m;
    bool ready = false;

    std::jthread client([&context, &out, client_sleep_time, &event_cv, &event_m, &ready](std::stop_token token)
    {
        //Wait until all the worker threads are started.
        {
            std::unique_lock<std::mutex> lock(event_m);

            event_cv.wait(lock, [&ready]()
            {
                return ready;
            });
        }

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

    //We can move this before std::jthread and got rid of event_cv.
    awl::StopWatch sw;

    std::vector<std::thread> v;
    v.reserve(thread_count);

    for (size_t i : awl::make_count(thread_count))
    {
        static_cast<void>(i);

        v.push_back(std::thread([&context, &out, &token, client_sleep_time, worker_sleep_time, &m, sw, &ex_ptr]()
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

                awl::sleep_for(Duration(worker_sleep_time), token);

                out(awl::format() << _T("Worker ") << std::this_thread::get_id() << _T(" has woken up within ") << sw);

                const auto elapsed = sw.GetElapsedCast<Duration>();

                //This assert failed until I moved StopWatch to the main thread.
                AWT_ASSERT(elapsed >= Duration(client_sleep_time));

                //Ctrl+Z on Linux causes this assertion to fail.
                AWT_ASSERT(elapsed < Duration(worker_sleep_time));

                out(awl::format() << _T("Worker ") << std::this_thread::get_id() << _T(" succeeded."));
            }
            catch (const std::exception&)
            {
                out(awl::format() << _T("Worker ") << std::this_thread::get_id() << _T(" failed."));

                if (ex_ptr == nullptr)
                {
                    ex_ptr = std::current_exception();
                }
            }
        }));
    }

    {
        std::unique_lock<std::mutex> lock(event_m);

        ready = true;
    }

    event_cv.notify_all();

    client.join();

    client.request_stop();

    for (auto& t : v)
    {
        t.join();
    }

    //We store only the first exception.
    if (ex_ptr != nullptr)
    {
        std::rethrow_exception(ex_ptr);
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

        awl::condition_variable_any().wait(lock, stoken, [&stoken] { return false; });

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
