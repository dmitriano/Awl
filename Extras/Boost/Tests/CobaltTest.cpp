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

namespace
{
    cobalt::generator<int> number_source(int count)
    {
        auto exec = co_await cobalt::this_coro::executor;
        asio::steady_timer timer(exec);

        for (int i = 1; i <= count; ++i)
        {
            timer.expires_after(500ms);
            co_await timer.async_wait(cobalt::use_op);
            co_yield i;
        }

        co_return 0;
    }

    using GenPtr = std::shared_ptr<cobalt::generator<int>>;

    cobalt::task<void> producer(cobalt::channel<int>& ch)
    {
        auto gen = number_source(5);

        BOOST_COBALT_FOR(auto val, gen)
        {
            std::cout << "Produced: " << val << "\n";
            co_await ch.write(val);  // асинхронно отправляем значение в канал
        }

        ch.close();  // обязательно закрыть канал, чтобы читатели завершились
        std::cout << "Producer finished\n";
        co_return;
    }

    cobalt::task<void> consumer(int id, GenPtr gen)
    {
        auto exec = co_await cobalt::this_coro::executor;
        asio::steady_timer timer(exec);

        while (true)
        {
            auto val = co_await *gen;
            if (!val)
                break;  // канал закрыт

            std::cout << "Consumer " << id << " got: " << val << "\n";

            timer.expires_after(500ms);
            co_await timer.async_wait(cobalt::use_op);
        }

        std::cout << "Consumer " << id << " finished\n";

        co_return;
    }
}

cobalt::main co_main(int argc, char* argv[])
{
    GenPtr gen = std::make_shared<cobalt::generator<int>>(number_source(10));
    
    auto exec = co_await cobalt::this_coro::executor;

    // cobalt::spawn(exec, producer(ch), boost::asio::detached);
    cobalt::spawn(exec, consumer(1, gen), boost::asio::detached);
    cobalt::spawn(exec, consumer(2, gen), boost::asio::detached);

    co_return 0;
}
