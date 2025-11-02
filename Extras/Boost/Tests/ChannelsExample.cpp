#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"

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
    struct Example
    {
        const awl::testing::TestContext& context;

        void spawn(asio::any_io_executor exec)
        {
            // Create a channel with capacity for 3 buffered messages
            asio::experimental::channel<void(boost::system::error_code, std::string)> ch(exec, 3);

            // Launch producer and consumer coroutines
            asio::co_spawn(exec, producer(ch), asio::detached);
            asio::co_spawn(exec, consumer(ch), asio::detached);
        }

        awaitable<void> producer(asio::experimental::channel<void(boost::system::error_code, std::string)>& ch)
        {
            for (int i = 1; i <= 5; ++i)
            {
                std::string msg = "Message " + std::to_string(i);
                context.logger.debug(awl::format() << "Producing: " << msg);

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
                for (;;)
                {
                    // Receive a message asynchronously from the channel
                    std::string msg = co_await ch.async_receive(asio::use_awaitable);
                    context.logger.debug(awl::format() << "Consumed: " << msg);
                }
            }
            catch (const boost::system::system_error& e)
            {
                // Check if the channel was closed gracefully
                if (e.code() == boost::asio::experimental::error::channel_closed)
                    context.logger.debug("Channel closed, exiting consumer");
                else
                    context.logger.debug(awl::format() << "Receive error: " << e.code().message());
            }
        }
    };
}

AWL_TEST(Channels)
{
    asio::io_context io;

    Example example{ context };

    example.spawn(io.get_executor());

    // Run the I/O context to process asynchronous operations
    io.run();
}
