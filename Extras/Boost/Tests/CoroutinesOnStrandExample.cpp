#include "Awl/StringFormat.h"
#include "Awl/Random.h"
#include "Awl/StopWatch.h"
#include "Awl/IntRange.h"
#include "Awl/Testing/UnitTest.h"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/strand.hpp>

#include <chrono>
#include <mutex>
#include <optional>
#include <random>
#include <string_view>
#include <thread>
#include <vector>
#include <atomic>

namespace asio = boost::asio;
using asio::awaitable;
using asio::use_awaitable;

namespace
{
    using Strand = asio::strand<asio::thread_pool::executor_type>;

    using Value = std::atomic<size_t>;

    class CoroutineWorker
    {
    public:

        explicit CoroutineWorker(const awl::testing::TestContext& context, 
            asio::any_io_executor executor, std::size_t index, Value& val, std::chrono::milliseconds work_duration)
        : 
            context(std::cref(context)),
            executor(executor),
            m_index(index),
            m_val(val),
            workDuration(work_duration)
        {}

        awaitable<void> run()
        {
            log("run started");

            co_await switchThread();

            log("run resumed before awaiting second");

            auto before = std::this_thread::get_id();

            auto value = co_await second();

            auto after = std::this_thread::get_id();

            logger().debug(awl::format()
                << "#" << m_index
                << " first awaited second on thread " << before
                << " and resumed on thread " << after
                << ", result = " << value);
        }

        awaitable<void> print()
        {
            log("run started");

            co_return;
        }

    private:

        void log(const char* caption) const
        {
            logger().debug(awl::format() << "#" << m_index << " " << caption << " on thread " << std::this_thread::get_id());
        }

        awaitable<int> second()
        {
            co_await switchThread();

            simulateWork();

            log("second resumed before awaiting third");

            auto value = co_await third();

            log("second resumed after awaiting third");

            co_return value * 2;
        }

        awaitable<int> third()
        {
            co_await switchThread();

            simulateWork();

            log("third resumed");

            co_return 2;
        }

        awaitable<void> switchThread()
        {
            co_await asio::post(getExecutor(), use_awaitable);
        }

        void simulateWork()
        {
            log("started a work");

            awl::StopWatch sw;

            for (size_t i = 0; ; ++i)
            {
                if (sw.HasElapsed(workDuration))
                {
                    break;
                }

                Value sample = i;

                m_val.store(sample);

                if (m_val.load() != sample)
                {
                    logger().error(awl::format() << "#" << m_index << " " << "Data Race!");
                }
            }

            log("finished the work");
        }

        awaitable<void> simulateWorkAsync() const
        {
            log("started a work");

            asio::steady_timer timer{ getExecutor() };

            timer.expires_after(workDuration);

            co_await timer.async_wait(use_awaitable);

            log("finished the work");
        }

        asio::any_io_executor getExecutor() const
        {
            return executor;
        }

        awl::Logger& logger() const
        {
            return context.get().logger;
        }

        std::reference_wrapper<const awl::testing::TestContext> context;
        asio::any_io_executor executor;
        const std::size_t m_index;
        Value& m_val;
        const std::chrono::milliseconds workDuration;
    };

    class StrandHolder
    {
    public:

        StrandHolder(const awl::testing::TestContext& context, asio::thread_pool& pool, bool use_strand) :
            context(context), pool(pool), strand(makeStrand(use_strand))
        {
            if (strand)
            {
                context.logger.debug("Using Strand.");
            }
            else
            {
                context.logger.debug("Using Thread Pool without a Strand.");
            }
        }

        asio::any_io_executor getExecutor() const
        {
            if (strand)
            {
                return *strand;
            }

            return pool.get_executor();
        }

    private:

        std::optional<Strand> makeStrand(bool use_strand)
        {
            if (use_strand)
            {
                return Strand{ pool.get_executor() };
            }
            else
            {
                return {};
            }
        }

        const awl::testing::TestContext& context;
        asio::thread_pool& pool;
        std::optional<Strand> strand;
    };

    void runWorkers(asio::thread_pool& pool, std::vector<CoroutineWorker>& workers)
    {
        for (CoroutineWorker& worker : workers)
        {
            asio::co_spawn(pool, worker.run(), asio::detached);
        }
    }

    void printWorkers(asio::thread_pool& pool, std::vector<CoroutineWorker>& workers)
    {
        for (CoroutineWorker& worker : workers)
        {
            asio::co_spawn(pool, worker.print(), asio::detached);
        }
    }
}

AWL_EXAMPLE(CoroutinesOnStrandExample)
{
    AWL_ATTRIBUTE(size_t, thread_count, std::max(1u, std::thread::hardware_concurrency()));

    // The number of concurrent workers.
    AWL_ATTRIBUTE(size_t, worker_count, 3);

    // Prints "Data Race!" if this flag is set.
    AWL_FLAG(without_strand);

    context.logger.debug(awl::format() << "Thread count: " << thread_count);

    asio::thread_pool pool(thread_count);

    StrandHolder holder{ context, pool, !without_strand };

    Value val;

    std::vector<CoroutineWorker> workers;

    AWL_ATTRIBUTE(int, duration_from, 200);
    AWL_ATTRIBUTE(int, duration_to, 1000);

    std::uniform_int_distribution<int> distribution(duration_from, duration_to);

    for (size_t i : awl::make_count(worker_count))
    {
        workers.push_back(CoroutineWorker{ context, holder.getExecutor(), i, val,
            std::chrono::milliseconds{distribution(awl::random())} });
    }

    runWorkers(pool, workers);

    pool.join();
}
