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

            if (ec)
                co_return ec;

            buffer.erase(0, n);
        }

        co_return asio::error::broken_pipe;
    }

    using GenReadPtr = std::shared_ptr<GenRead>;

    cobalt::task<void> read_consumer(int id, GenReadPtr gen)
    {
        if (id == 1)
        {
            auto exec = co_await cobalt::this_coro::executor;

            asio::steady_timer timer(exec);

            timer.expires_after(500ms);

            co_await timer.async_wait(cobalt::use_op);
        }

        BOOST_COBALT_FOR( // would be for co_await(auto value : read_lines(sf)) if standardized
            auto line,
            *gen)
        {
            if (line.has_error() && line.error() != asio::error::eof)
                std::cerr << "Consumer " << id << " Error occured: " << line.error() << std::endl;
            else if (line.has_value())
                std::cout << "Consumer " << id << " Read line '" << *line << "'" << std::endl;
        }

        std::cout << "Consumer " << id << " finished\n";

        co_return;
    }

    cobalt::task<void> read_consumer2(int id, GenReadPtr gen)
    {
        if (id == 1)
        {
            auto exec = co_await cobalt::this_coro::executor;

            asio::steady_timer timer(exec);

            timer.expires_after(500ms);

            co_await timer.async_wait(cobalt::use_op);
        }

        while (true)
        {
            auto line = co_await *gen;

            if (line.has_error())
            {
                std::cerr << "Consumer " << id << " Error occured: " << line.error() << std::endl;

                break;
            }
            else if (line.has_value())
            {
                std::cout << "Consumer " << id << " Read line '" << *line << "'" << std::endl;
            }
        }


        std::cout << "Consumer " << id << " finished\n";

        co_return;
    }
}

cobalt::main co_main(int argc, char* argv[])
{
    asio::stream_file sf{ co_await cobalt::this_coro::executor,
                         argv[1], // skipping the check here for brevity.
                         asio::stream_file::read_only };

    GenReadPtr gen = std::make_shared<GenRead>(read_lines(sf));

    auto t1 = read_consumer(1, gen);
    auto t2 = read_consumer(2, gen);

    co_await t1;
    co_await t2;

    //cobalt::spawn(exec, read_consumer(1, gen), boost::asio::detached);
    //cobalt::spawn(exec, read_consumer(2, gen), boost::asio::detached);

    co_return 0;
}
