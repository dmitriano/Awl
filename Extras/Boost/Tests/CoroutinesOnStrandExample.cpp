#include "Awl/StringFormat.h"
#include "Awl/Random.h"
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

namespace asio = boost::asio;
using asio::awaitable;
using asio::use_awaitable;

namespace
{
    using Strand = asio::strand<asio::thread_pool::executor_type>;

    class CoroutineChain
    {
    public:

        explicit CoroutineChain(const awl::testing::TestContext& context, asio::any_io_executor executor)
            : context(context), executor(executor)
        {
        }

        awaitable<void> first()
        {
            log("first started");

            co_await switchThread();

            log("first resumed before awaiting second");

            auto before = std::this_thread::get_id();

            auto value = co_await second();

            auto after = std::this_thread::get_id();

            context.logger.debug(awl::format()
                << "first awaited second on thread " << before
                << " and resumed on thread " << after
                << ", result = " << value);
        }

    private:

        void log(const char* caption) const
        {
            context.logger.debug(awl::format() << caption << " on thread " << std::this_thread::get_id());
        }

        awaitable<int> second()
        {
            co_await switchThread();

            co_await simulateWork();

            log("second resumed before awaiting third");

            auto value = co_await third();

            log("second resumed after awaiting third");

            co_return value * 2;
        }

        awaitable<int> third()
        {
            co_await switchThread();

            co_await simulateWork();

            log("third resumed");

            co_return 2;
        }

        awaitable<void> switchThread()
        {
            co_await asio::post(getExecutor(), use_awaitable);
        }

        awaitable<void> simulateWork() const
        {
            asio::steady_timer timer{ getExecutor() };

            timer.expires_after(randomDuration());

            co_await timer.async_wait(use_awaitable);
        }

        static std::chrono::milliseconds randomDuration()
        {
            static thread_local std::uniform_int_distribution<int> distribution(0, 200);

            return std::chrono::milliseconds(distribution(awl::random()));
        }

        asio::any_io_executor getExecutor() const
        {
            return executor;
        }

        const awl::testing::TestContext& context;
        asio::any_io_executor executor;
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
}

AWL_EXAMPLE(CoroutinesOnStrandExample)
{
    AWL_ATTRIBUTE(size_t, thread_count, 5);
    AWL_FLAG(without_strand);

    asio::thread_pool pool(thread_count);

    StrandHolder holder{ context, pool, !without_strand };

    CoroutineChain chain{ context, holder.getExecutor() };

    asio::co_spawn(pool, chain.first(), asio::detached);

    pool.join();
}
