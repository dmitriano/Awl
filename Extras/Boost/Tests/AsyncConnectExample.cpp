#include "Awl/StringFormat.h"
#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/experimental/promise.hpp>
#include <boost/asio/experimental/use_promise.hpp>
#include <boost/asio/ssl.hpp>
#include <random>
using error_code = boost::system::error_code;
using namespace std::chrono_literals;
namespace asio = boost::asio;
using asio::ip::tcp;

namespace
{
    class SocketWrapper {
    private:
        using SslStream = asio::ssl::stream<tcp::socket>;

    public:

        using Sig = void(error_code, size_t);

        SocketWrapper(asio::any_io_executor ex) : s_{ ex, ctx_ } {}

        // Token can be asio::completion_token_for<Sig>.
        template <typename Token = asio::deferred_t>
        auto asyncConnect(tcp::resolver::results_type eps, Token&& token = {}) {
            auto op = asio::async_connect( //
                underlyingSocket(), std::move(eps), asio::deferred([this](error_code ec, tcp::endpoint) {
                    return asio::deferred
                        .when(ec.failed() || !isSSL())
                        .then(asio::deferred.values(ec))
                        .otherwise(stream().async_handshake(SslStream::client));
                    }));

            return std::move(op)(std::forward<Token>(token));
        }

    private:

        bool isSSL() const { return true; }

        tcp::socket const& underlyingSocket() const { return s_.next_layer(); }
        tcp::socket& underlyingSocket() { return s_.next_layer(); }
        SslStream const& stream() const { return s_; }
        SslStream& stream() { return s_; }

        asio::ssl::context ctx_{ asio::ssl::context::tlsv13 };
        SslStream          s_;
    };
}

AWL_EXAMPLE(AsyncConnect)
{
    AWL_ATTRIBUTE(std::string, target, "192.168.0.123:389");
    AWL_ATTRIBUTE(int, select, std::random_device{}());

    auto pos = target.find(':');
    const std::string target_host = target.substr(0, pos);
    const std::string target_port = target.substr(pos + 1);

    asio::thread_pool ioc(1);

    SocketWrapper w(ioc.get_executor());
    auto eps = tcp::resolver{ ioc }.resolve(target_host, target_port);

    switch (select % 4)
    {
    case 0:
        context.logger.debug("Straight callback");
        w.asyncConnect(eps, [&context](error_code ec) { context.logger.debug(awl::format() << "Callback: " << ec.message()); });
        break;
    case 1:
        context.logger.debug("Coro await");
        asio::co_spawn(ioc, [&] -> asio::awaitable<void>
        {
            co_await w.asyncConnect(eps);
            context.logger.debug("Coro connected");
        },
        asio::detached);
        break;
    case 2:
        context.logger.debug("Coro await with promise");
        asio::co_spawn(ioc, [&] -> asio::awaitable<void>
        {
            auto p = w.asyncConnect(eps, asio::experimental::use_promise);
            context.logger.debug("Doing some other time consuming stuff as well");

            co_await p(asio::deferred);
            context.logger.debug("Coro with promise connected");
        },
        asio::detached);
        break;
    case 3: {
        context.logger.debug("Custom completion token with adaptors");
        auto f = w.asyncConnect(eps, asio::as_tuple(asio::use_future));

        if (f.wait_for(20ms) == std::future_status::ready) {
            auto [ec] = f.get();
            context.logger.debug(awl::format() << "Future resolved within 20ms: " << ec.message());
        }
        break;
    }
    };

    ioc.join();
}
