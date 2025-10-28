#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <exception>
#include <type_traits>

namespace asio = boost::asio;
using boost::system::error_code;

namespace
{
    // -------------------------------------------
    // Bridge: awaitable<T>  ---> async-style handler: void(error_code, T)
    // -------------------------------------------
    template <class T, class CompletionToken>
    auto async_from_awaitable(asio::any_io_executor ex,
        asio::awaitable<T> aw,
        CompletionToken&& token)
    {
        using token_type = std::decay_t<CompletionToken>;

        // Materialize the token as an lvalue to satisfy async_initiate on MSVC
        token_type tk(std::forward<CompletionToken>(token));

        return asio::async_initiate<token_type, void(error_code, T)>(
            [ex, aw = std::move(aw)](auto&& handler) mutable {
                // Spawn the coroutine; on completion, deliver via the handler
                asio::co_spawn(
                    ex,
                    std::move(aw),
                    // co_spawn completion for non-void: void(std::exception_ptr, T)
                    [h = std::forward<decltype(handler)>(handler)]
                    (std::exception_ptr ep, T value) mutable
                    {
                        if (ep) {
                            try {
                                std::rethrow_exception(ep);
                            }
                            catch (const boost::system::system_error& se) {
                                std::move(h)(se.code(), T{});
                                return;
                            }
                            catch (...) {
                                std::move(h)(
                                    make_error_code(boost::system::errc::operation_canceled), T{});
                                return;
                            }
                        }
                        std::move(h)({}, std::move(value));
                    }
                );
            },
            tk // pass as lvalue
        );
    }

    asio::awaitable<int> compute_answer()
    {
        co_return 42;
    }
}

AWL_EXAMPLE(CallbackFromAwaitable)
{
    AWL_UNUSED_CONTEXT;

    asio::io_context io;
    auto ex = io.get_executor();

    // Call awaitable<T> with a callback handler
    async_from_awaitable(ex, compute_answer(),
        // Handler signature matches void(error_code, int)
        [](error_code ec, int value) {
            if (ec) {
                std::cerr << "Error: " << ec.message() << "\n";
            }
            else {
                std::cout << "Value: " << value << "\n";
            }
        }
    );

    io.run();
}
