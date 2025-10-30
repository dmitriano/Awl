#include "Awl/StringFormat.h"
#include "Awl/Testing/UnitTest.h"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/strand.hpp>

#include <mutex>
#include <string_view>
#include <thread>

namespace asio = boost::asio;
using asio::awaitable;
using asio::use_awaitable;

namespace
{
    class CoroutineChain
    {
    public:

        using Strand = asio::strand<asio::thread_pool::executor_type>;

        explicit CoroutineChain(const awl::testing::TestContext& context, asio::thread_pool& pool, bool use_strand)
            : context(context), pool(pool), strand{ makeStrand(use_strand)}
        {}

        void log(const char* caption) const
        {
            context.logger.debug(awl::format() << caption << " on thread " << std::this_thread::get_id());
        }

        awaitable<int> third()
        {
            co_await switchThread();

            log("third resumed");

            co_return 2;
        }

        awaitable<int> second()
        {
            co_await switchThread();

            log("second resumed before awaiting third");

            auto value = co_await third();

            log("second resumed after awaiting third");

            co_return value * 2;
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

        std::optional<Strand> makeStrand(bool use_strand) const
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

        awaitable<void> switchThread()
        {
            if (strand)
            {
                co_await asio::post(*strand, use_awaitable);
            }
            else
            {
                co_await asio::post(pool, use_awaitable);
            }
        }

        const awl::testing::TestContext& context;
        asio::thread_pool& pool;
        std::optional<Strand> strand;
    };
}

AWL_EXAMPLE(AsioThreadPoolCoroutine)
{
    AWL_FLAG(use_strand);

    asio::thread_pool pool(5);

    CoroutineChain chain{context, pool, use_strand };

    asio::co_spawn(pool, chain.first(), asio::detached);

    pool.join();
}
