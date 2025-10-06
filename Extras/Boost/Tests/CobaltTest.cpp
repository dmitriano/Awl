/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/asio.hpp>
#include <boost/cobalt.hpp>
#include <iostream>
#include <chrono>
#include <generator>

namespace asio = boost::asio;
namespace cobalt = boost::cobalt;
using namespace std::chrono_literals;

std::generator<int> numbers()
{
    for (int i = 1; i <= 5; ++i)
    {
        co_yield i;
    }
}

cobalt::generator<int> numbers2()
{
    for (int i = 1; i <= 5; ++i)
    {
        // co_await cobalt::this_coro::sleep_for(std::chrono::milliseconds(200));
        
        co_yield i;
    }

    co_return 10;
}

cobalt::generator<int> numbers3(asio::any_io_executor exec)
{
    asio::steady_timer timer(exec);

    for (int i = 1; i <= 5; ++i)
    {
        timer.expires_after(500ms);
        co_await timer.async_wait(cobalt::use_op);
        co_yield i;
    }

    co_return 0;
}

cobalt::main co_main(int argc, char* argv[])
{
    auto exec = co_await cobalt::this_coro::executor;

    auto gen = numbers3(exec);

    BOOST_COBALT_FOR(auto val, gen)
    {
        std::cout << "Got value: " << val << std::endl;
    }

    co_return 0;
}
