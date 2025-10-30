#include "Awl/StringFormat.h"
#include "Awl/Testing/UnitTest.h"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <mutex>
#include <string_view>
#include <thread>

using boost::asio::awaitable;
using boost::asio::use_awaitable;

namespace
{
    struct CoroutineChain
    {
        boost::asio::thread_pool& pool;
        const awl::testing::TestContext& context;

        explicit CoroutineChain(boost::asio::thread_pool& pool, const awl::testing::TestContext& context)
            : pool(pool), context(context)
        {
        }

        void log(const char* caption) const
        {
            context.logger.debug(awl::format() << caption << " on thread " << std::this_thread::get_id());
        }

        awaitable<int> third()
        {
            co_await boost::asio::post(pool, use_awaitable);

            log("third resumed");

            co_return 2;
        }

        awaitable<int> second()
        {
            co_await boost::asio::post(pool, use_awaitable);

            log("second resumed before awaiting third");

            auto value = co_await third();

            log("second resumed after awaiting third");

            co_return value * 2;
        }

        awaitable<void> first()
        {
            log("first started");

            co_await boost::asio::post(pool, use_awaitable);

            log("first resumed before awaiting second");

            auto before = std::this_thread::get_id();

            auto value = co_await second();

            auto after = std::this_thread::get_id();

            context.logger.debug(awl::format()
                << "first awaited second on thread " << before
                << " and resumed on thread " << after
                << ", result = " << value);
        }
    };
}

AWL_EXAMPLE(AsioThreadPoolCoroutine)
{
    boost::asio::thread_pool pool(5);

    CoroutineChain chain{pool, context};

    boost::asio::co_spawn(pool, chain.first(), boost::asio::detached);

    pool.join();
}
