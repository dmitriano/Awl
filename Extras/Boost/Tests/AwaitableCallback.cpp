#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <iostream>

namespace asio = boost::asio;
using boost::system::error_code;

namespace
{
    class LegacyProcessor
    {
    public:

        LegacyProcessor(asio::any_io_executor executor) :
            m_executor(std::move(executor))
        {}

        // -------------------------------------------
        // Legacy-style async operation (no std::function!)
        // Accepts any completion handler — can be move-only
        // -------------------------------------------
        template <class CompletionHandler>
        void asyncProcess(CompletionHandler&& handler)
        {
            // Simulate asynchronous work by posting the handler
            asio::post(m_executor,
                [h = std::forward<CompletionHandler>(handler)]() mutable {
                    // Call the handler once the "operation" is complete
                    std::move(h)(error_code{}, 42);
                });
        }

        template <class CompletionHandler>
        void asyncProcessOnThread(CompletionHandler&& handler)
        {
            std::thread t([this, h = std::forward<CompletionHandler>(handler)]() mutable
                {
                    static_assert(std::is_same_v<decltype(h), CompletionHandler>);

                    asyncProcess(std::move(h));
                });

            t.join();
        }

    private:

        asio::any_io_executor m_executor;
    };

    class ModernProcessor
    {
    public:

        ModernProcessor(asio::any_io_executor executor) :
            m_processor(std::move(executor))
        {}

        // -------------------------------------------
        // Adapter: convert async_legacy_op → awaitable<int>
        // -------------------------------------------
        asio::awaitable<int> asyncProcess()
        {
            // async_initiate turns callback-style API into co_await-compatible awaitable
            co_return co_await asio::async_initiate<
                decltype(asio::use_awaitable),
                void(error_code, int)
            >(
                // Initiator: calls the legacy function with the handler provided by Asio
                [this](auto&& completion_handler) {
                    m_processor.asyncProcess(std::forward<decltype(completion_handler)>(completion_handler));
                },
                asio::use_awaitable
            );
        }

        asio::awaitable<int> asyncProcessOnThread()
        {
            // async_initiate turns callback-style API into co_await-compatible awaitable
            co_return co_await asio::async_initiate<
                decltype(asio::use_awaitable),
                void(error_code, int)
            >(
                // Initiator: calls the legacy function with the handler provided by Asio
                [this](auto&& completion_handler) {
                    m_processor.asyncProcessOnThread(std::forward<decltype(completion_handler)>(completion_handler));
                },
                asio::use_awaitable
            );
        }

    private:

        LegacyProcessor m_processor;
    };

    // -------------------------------------------
    // Example coroutine using the awaitable wrapper
    // -------------------------------------------
    asio::awaitable<void> example()
    {
        // Get the current coroutine executor
        auto exec = co_await asio::this_coro::executor;

        ModernProcessor processor(exec);

        {
            int v = co_await processor.asyncProcess();

            std::cout << "Result = " << v << "\n";
        }

        {
            int v = co_await processor.asyncProcessOnThread();

            std::cout << "Result = " << v << "\n";
        }
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
