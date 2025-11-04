#include "Awl/Testing/UnitTest.h"
#include "Awl/String.h"
#include "Awl/StringFormat.h"

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
using namespace std::chrono_literals;
auto now = std::chrono::steady_clock::now;
static auto start = now();

using namespace boost::asio::experimental::awaitable_operators;
using boost::asio::awaitable;
using boost::asio::use_awaitable;
using boost::system::error_code;


awaitable<void> foo_and()
{
    boost::asio::steady_timer tim1(co_await boost::asio::this_coro::executor, 100ms);
    boost::asio::steady_timer tim2(co_await boost::asio::this_coro::executor, 200ms);

    co_await(tim1.async_wait(use_awaitable) && tim2.async_wait(use_awaitable));
}

awaitable<void> foo_or()
{
    boost::asio::steady_timer tim1(co_await boost::asio::this_coro::executor, 100ms);
    boost::asio::steady_timer tim2(co_await boost::asio::this_coro::executor, 200ms);

    co_await(tim1.async_wait(use_awaitable) || tim2.async_wait(use_awaitable));
}

AWL_TEST(AsioAwaitableOperators)
{
    boost::asio::thread_pool ioc;

    auto make_handler = [&context](std::string caption)
    {
        return [=](std::exception_ptr e)
            {
                using awl::operator <<;

                const char* status = "succeeded";

                try
                {
                    if (e)
                        std::rethrow_exception(e);
                }
                catch (std::exception const&)
                {
                    status = "failed";
                }

                context.logger.debug(awl::format() << caption << ' ' << status << " at " << (now() - start) / 1.0ms << "ms");
            };
    };

    boost::asio::co_spawn(ioc.get_executor(), foo_and(), make_handler("foo_and"));
    boost::asio::co_spawn(ioc.get_executor(), foo_or(), make_handler("foo_or"));

    ioc.join();
}
