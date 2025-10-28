#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/experimental/channel_error.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>

namespace asio = boost::asio;
using asio::awaitable;
using asio::use_awaitable;
using namespace std::chrono_literals;

namespace
{
    // Пример с каналом, передающим строки
    awaitable<void> producer(asio::experimental::channel<void(boost::system::error_code, std::string)>& ch)
    {
        for (int i = 1; i <= 5; ++i)
        {
            std::string msg = "Message " + std::to_string(i);
            std::cout << "Producing: " << msg << std::endl;
            co_await ch.async_send({}, msg, use_awaitable);
            co_await asio::steady_timer(co_await asio::this_coro::executor, 500ms).async_wait(use_awaitable);
        }

        // Закрываем канал, чтобы получатель знал, что сообщений больше не будет
        ch.close();
    }

    awaitable<void> consumer(asio::experimental::channel<void(boost::system::error_code, std::string)>& ch)
    {
        try {
            for (;;) {
                std::string msg = co_await ch.async_receive(asio::use_awaitable);
                std::cout << "Consumed: " << msg << '\n';
            }
        }
        catch (const boost::system::system_error& e) {
            if (e.code() == boost::asio::experimental::error::channel_closed)
                std::cout << "Channel closed, exiting consumer.\n";
            else
                std::cout << "Receive error: " << e.code().message() << '\n';
        }
    }
}

AWL_EXAMPLE(Channels)
{
    AWL_UNUSED_CONTEXT;

    asio::io_context io;

    // Канал с буфером на 3 сообщения
    asio::experimental::channel<void(boost::system::error_code, std::string)> ch(io, 3);

    asio::co_spawn(io, producer(ch), asio::detached);
    asio::co_spawn(io, consumer(ch), asio::detached);

    io.run();
}
