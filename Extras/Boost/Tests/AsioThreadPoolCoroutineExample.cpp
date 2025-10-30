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

        explicit CoroutineChain(const awl::testing::TestContext& context, asio::thread_pool& pool)
            : context(context), pool(pool), strand(pool.get_executor())
        {
        }

        void log(const char* caption) const
        {
            context.logger.debug(awl::format() << caption << " on thread " << std::this_thread::get_id());
        }

        awaitable<int> third()
        {
            co_await asio::post(pool, use_awaitable);

            log("third resumed");

            co_return 2;
        }

        awaitable<int> second()
        {
            co_await asio::post(pool, use_awaitable);

            log("second resumed before awaiting third");

            auto value = co_await third();

            log("second resumed after awaiting third");

            co_return value * 2;
        }

        awaitable<void> first()
        {
            log("first started");

            co_await asio::post(pool, use_awaitable);

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

        const awl::testing::TestContext& context;
        asio::thread_pool& pool;
        asio::strand<asio::thread_pool::executor_type> strand;
    };
}

AWL_EXAMPLE(AsioThreadPoolCoroutine)
{
    asio::thread_pool pool(5);

    CoroutineChain chain{context, pool};

    asio::co_spawn(pool, chain.first(), asio::detached);

    pool.join();
}
