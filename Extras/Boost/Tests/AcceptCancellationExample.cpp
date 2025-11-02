#include "Awl/StringFormat.h"
#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <chrono>

namespace asio = boost::asio;
using asio::awaitable;
using asio::use_awaitable;
using tcp = asio::ip::tcp;
using namespace std::chrono_literals;

namespace
{
    // Coroutine that waits for a client connection but can be cancelled
    awaitable<tcp::socket> accept_with_cancellation(tcp::acceptor& acceptor,
        asio::cancellation_signal& cancel_signal,
        const awl::testing::TestContext& context)
    {
        auto ex = co_await asio::this_coro::executor;

        context.logger.debug(awl::format() << "Waiting for a client on " << acceptor.local_endpoint() << "...");

        // Bind the external cancellation slot specifically to async_accept.
        // In Boost 1.89 this is the correct way to hook your own signal.
        tcp::socket socket =
            co_await acceptor.async_accept(
                asio::bind_cancellation_slot(cancel_signal.slot(), use_awaitable));

        context.logger.debug(awl::format() << "Client connected from " << socket.remote_endpoint());

        co_return socket;
    }

    // Cancels after 3 seconds by emitting on the shared signal
    awaitable<void> cancel_after_delay(asio::cancellation_signal& cancel_signal,
        const awl::testing::TestContext& context)
    {
        auto ex = co_await asio::this_coro::executor;
        asio::steady_timer t(ex);

        t.expires_after(3s);
        co_await t.async_wait(use_awaitable);

        context.logger.debug("No client yet — sending cancellation signal...");
        cancel_signal.emit(asio::cancellation_type::all);
    }

    awaitable<void> example(const awl::testing::TestContext& context)
    {
        auto ex = co_await asio::this_coro::executor;

        tcp::acceptor acc(ex, tcp::endpoint(tcp::v4(), 5555));
        asio::cancellation_signal sig;

        // After 3s, emit cancellation (will abort async_accept)
        asio::co_spawn(ex, cancel_after_delay(sig, context), asio::detached);

        try
        {
            // Run accept coroutine in parallel
            tcp::socket socket = co_await accept_with_cancellation(acc, sig, context);
        }
        catch (const boost::system::system_error& e)
        {
            // async_accept completes with operation_aborted when cancelled
            if (e.code() == asio::error::operation_aborted)
                context.logger.debug("Accept operation was cancelled.");
            else
                throw;
        }
    }
}

AWL_EXAMPLE(AcceptCancellation)
{
    AWL_UNUSED_CONTEXT;

    try
    {
        asio::io_context ctx;
        asio::co_spawn(ctx, example(context), asio::detached);
        ctx.run();
    }
    catch (const std::exception& e)
    {
        context.logger.error(awl::format() << "Exception: " << e.what());
    }
}
