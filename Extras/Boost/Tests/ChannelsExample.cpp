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
    // Example of a channel that transfers strings between coroutines
    awaitable<void> producer(asio::experimental::channel<void(boost::system::error_code, std::string)>& ch)
    {
        for (int i = 1; i <= 5; ++i)
        {
            std::string msg = "Message " + std::to_string(i);
            std::cout << "Producing: " << msg << std::endl;

            // Send the message asynchronously to the channel
            co_await ch.async_send({}, msg, use_awaitable);

            // Wait for 500 ms before sending the next message
            co_await asio::steady_timer(co_await asio::this_coro::executor, 500ms).async_wait(use_awaitable);
        }

        // Close the channel to signal that no more messages will be sent
        ch.close();
    }

    // Coroutine that receives messages from the channel
    awaitable<void> consumer(asio::experimental::channel<void(boost::system::error_code, std::string)>& ch)
    {
        try {
            for (;;) {
                // Receive a message asynchronously from the channel
                std::string msg = co_await ch.async_receive(asio::use_awaitable);
                std::cout << "Consumed: " << msg << '\n';
            }
        }
        catch (const boost::system::system_error& e) {
            // Check if the channel was closed gracefully
            if (e.code() == boost::asio::experimental::error::channel_closed)
                std::cout << "Channel closed, exiting consumer.\n";
            else
                std::cout << "Receive error: " << e.code().message() << '\n';
        }
    }
}

// Example test case using AWL's testing framework
AWL_TEST(Channels)
{
    AWL_UNUSED_CONTEXT;

    asio::io_context io;

    // Create a channel with capacity for 3 buffered messages
    asio::experimental::channel<void(boost::system::error_code, std::string)> ch(io, 3);

    // Launch producer and consumer coroutines
    asio::co_spawn(io, producer(ch), asio::detached);
    asio::co_spawn(io, consumer(ch), asio::detached);

    // Run the I/O context to process asynchronous operations
    io.run();
}
