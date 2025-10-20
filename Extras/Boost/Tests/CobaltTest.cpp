/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/asio.hpp>
#include <boost/cobalt.hpp>
#include <boost/algorithm/string.hpp>
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

//cobalt::main co_main(int argc, char* argv[])
//{
//    GenPtr gen = std::make_shared<cobalt::generator<int>>(number_source(10));
//    
//    auto exec = co_await cobalt::this_coro::executor;
//
//    // cobalt::spawn(exec, producer(ch), boost::asio::detached);
//    cobalt::spawn(exec, consumer(1, gen), boost::asio::detached);
//    cobalt::spawn(exec, consumer(2, gen), boost::asio::detached);
//
//    co_return 0;
//}

namespace
{
    using GenRead = cobalt::generator<boost::system::result<std::string>>;

    GenRead read_lines(asio::stream_file& f)
    {
        std::string buffer;
        while (f.is_open())
        {
            auto [ec, n] = co_await
                asio::async_read_until(f, asio::dynamic_buffer(buffer), '\n',
                    asio::as_tuple(cobalt::use_op));

            // no need to copy, just point to the buffer
            std::string_view ln{ buffer.c_str(), n }; // -1 to skip the line
            ln = boost::algorithm::trim_copy(ln);

            if (!ln.empty())
                co_yield ln;

            // Why we do not throw the error as an exception?
            if (ec)
                co_return ec;

            buffer.erase(0, n);
        }

        co_return asio::error::broken_pipe;
    }

    cobalt::generator<std::string> read_lines(const char* name, int n) {
        for (int i = 0; i < n; ++i) {
            co_yield std::string(name) + ":" + std::to_string(i);
        }
        co_return {};
    }
}

//cobalt::main co_main(int argc, char* argv[])
//{
//    auto ga = read_lines("a", 1000);
//    auto gb = read_lines("b", 1000);
//
//    while (ga || gb) { // оператор bool у cobalt::generator
//        auto v = co_await cobalt::race(ga, gb); // кто-то из генераторов готов — берём строку
//        boost::variant2::visit([](auto& s) { std::cout << s << "\n"; }, v);
//    }
//
//    co_return 0;
//}

cobalt::main co_main(int argc, char* argv[])
{
    asio::stream_file a{ co_await cobalt::this_coro::executor, argv[1], asio::stream_file::read_only };
    asio::stream_file b{ co_await cobalt::this_coro::executor, argv[2], asio::stream_file::read_only };

    auto ga = read_lines(a);
    auto gb = read_lines(b);

    //while (ga || gb) { // оператор bool у cobalt::generator
    //    auto v = co_await cobalt::race(ga, gb); // кто-то из генераторов готов — берём строку
    //    boost::variant2::visit([](auto& s) { std::cout << s << "\n"; }, v);
    //}

    try
    {
        // cobalt::race will crash if one of the generators is finished.
        while (ga || gb) {
            auto v = co_await cobalt::race(ga, gb); // кто-то из генераторов готов — берём строку
            boost::variant2::visit([](auto& s) { std::cout << s << "\n"; }, v);
        }

        // continue the interation without cobalt::race
        while (ga) {
            auto s = co_await ga;
            std::cout << s << '\n';
        }
        while (gb) {
            auto s = co_await gb;
            std::cout << s << '\n';
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    co_return 0;
}
