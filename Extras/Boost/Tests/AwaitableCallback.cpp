#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <iostream>

namespace asio = boost::asio;
using boost::system::error_code;

namespace
{
    // -------------------------------------------
    // Legacy-style async operation (no std::function!)
    // Accepts any completion handler — can be move-only
    // -------------------------------------------
    template <class CompletionHandler>
    void async_legacy_op(CompletionHandler&& handler)
    {
        // Simulate asynchronous work by posting the handler
        asio::post(asio::system_executor{},
            [h = std::forward<CompletionHandler>(handler)]() mutable {
                // Call the handler once the "operation" is complete
                std::move(h)(error_code{}, 42);
            });
    }

    // -------------------------------------------
    // Adapter: convert async_legacy_op → awaitable<int>
    // -------------------------------------------
    asio::awaitable<int> async_legacy_op_awaitable()
    {
        // async_initiate turns callback-style API into co_await-compatible awaitable
        co_return co_await asio::async_initiate<
            decltype(asio::use_awaitable),
            void(error_code, int)
        >(
            // Initiator: calls the legacy function with the handler provided by Asio
            [](auto&& completion_handler) {
                async_legacy_op(std::forward<decltype(completion_handler)>(completion_handler));
            },
            asio::use_awaitable
        );
    }

    // -------------------------------------------
    // Example coroutine using the awaitable wrapper
    // -------------------------------------------
    asio::awaitable<void> example()
    {
        int v = co_await async_legacy_op_awaitable();
        std::cout << "Result = " << v << "\n";
    }
}

AWL_EXAMPLE(AwaitableCallback)
{
    AWL_UNUSED_CONTEXT;

    asio::io_context io;

    // Launch the coroutine
    asio::co_spawn(io, example(), asio::detached);

    // Run the I/O context until all operations complete
    io.run();
}
