#include "Awl/StringFormat.h"
#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/experimental/promise.hpp>
#include <boost/asio/experimental/use_promise.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <iomanip>

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = asio::ip::tcp;

namespace
{
    asio::awaitable<void> transfer(
        ssl::stream<tcp::socket>& from,
        ssl::stream<tcp::socket>& to,
        const awl::testing::TestContext& context)
    {
        try
        {
            char buf[8192];
            while (true)
            {
                std::size_t n = co_await from.async_read_some(asio::buffer(buf), asio::use_awaitable);
                co_await asio::async_write(to, asio::buffer(buf, n), asio::use_awaitable);
            }
        }
        catch (const boost::system::system_error& e)
        {
            if (e.code() == boost::asio::ssl::error::stream_truncated)
            {
                // Normal termination: SSL shutdown was not sent
                context.logger.error("transfer: connection closed (stream truncated)");
            }
            else if (e.code() == boost::asio::error::eof)
            {
                context.logger.error("transfer: EOF");
            }
            else if (e.code() == asio::error::connection_reset)
            {
                context.logger.error("transfer: Connection Reset.");
            }
            else if (
                e.code() == boost::system::errc::operation_canceled
                || e.code() == boost::asio::error::operation_aborted)
            {
                context.logger.error("transfer: Operation cancelled.");
            }
            else if (e.code().category() == boost::system::system_category() && e.code().value() == 10054)
            {
                context.logger.error("transfer Windows Error: WSAECONNRESET.");
            }
            else
            {
                context.logger.error(awl::format() << "transfer boost::system::system_error: " << e.what());
            }
        }
        catch (std::exception& e)
        {
            // Any exception = stop proxying
            context.logger.error(awl::format() << "transfer std::exception: " << e.what());
        }
    }

    asio::awaitable<void> bidirectional_transfer(
        ssl::stream<tcp::socket>& client_ssl,
        ssl::stream<tcp::socket>& server_ssl,
        const awl::testing::TestContext& context)
    {
        using namespace boost::asio::experimental::awaitable_operators;

        // it calls wait_for_one_error()
        co_await(transfer(client_ssl, server_ssl, context) && transfer(server_ssl, client_ssl, context));
    }
        
    [[maybe_unused]]
    asio::awaitable<void> advanced_bidirectional_transfer_example(
        ssl::stream<tcp::socket>& client_ssl,
        ssl::stream<tcp::socket>& server_ssl,
        const awl::testing::TestContext& context)
    {
        auto ex = co_await boost::asio::this_coro::executor;

        auto [order, ex0, ex1] =
            co_await asio::experimental::make_parallel_group(
                asio::co_spawn(ex, transfer(client_ssl, server_ssl, context), boost::asio::deferred),
                asio::co_spawn(ex, transfer(server_ssl, client_ssl, context), boost::asio::deferred)
            ).async_wait(
                asio::experimental::wait_for_one_success(),
                asio::deferred
            );

        // At this point one of the tasks is still is in progress
        // and we wait it to finish here.
    }

    // Handling a single client
    asio::awaitable<void> handle_client(
        ssl::stream<tcp::socket> client_ssl,
        const std::string& target_host,
        const std::string& target_port,
        const awl::testing::TestContext& context)
    {
        try
        {
            // TLS handshake with the client (proxy acts as a server)
            co_await client_ssl.async_handshake(ssl::stream_base::server, asio::use_awaitable);

            // Get the current coroutine executor
            auto exec = co_await asio::this_coro::executor;

            // Create an SSL client for the target server
            tcp::resolver resolver(exec);
            auto endpoints = co_await resolver.async_resolve(target_host, target_port, asio::use_awaitable);

            ssl::context server_ctx(ssl::context::tlsv12_client);
            server_ctx.set_default_verify_paths();

            ssl::stream<tcp::socket> server_ssl(exec, server_ctx);
            co_await asio::async_connect(server_ssl.next_layer(), endpoints, asio::use_awaitable);
            co_await server_ssl.async_handshake(ssl::stream_base::client, asio::use_awaitable);

            co_await bidirectional_transfer(client_ssl, server_ssl, context);
        }
        catch (const boost::system::system_error& e)
        {
            if (e.code() == boost::asio::ssl::error::stream_truncated)
            {
                // Normal termination: SSL shutdown was not sent
                context.logger.error("handle_client: connection closed (stream truncated)");
            }
            else
            {
                context.logger.error(awl::format() << "handle_client exception: " << e.what());
            }
        }
        catch (const std::exception& e)
        {
            context.logger.error(awl::format() << "handle_client exception: " << e.what());
        }
    }

    asio::awaitable<void> runProxy(tcp::endpoint listen_endpoint, ssl::context client_ctx,
        const std::string& target_host, const std::string& target_port,
        const awl::testing::TestContext& context)
    {
        try
        {
            // Get the executor for the acceptor
            auto exec = co_await asio::this_coro::executor;
            tcp::acceptor acceptor(exec, listen_endpoint);

            while (true)
            {
                tcp::socket sock = co_await acceptor.async_accept(asio::use_awaitable);
                ssl::stream<tcp::socket> client_ssl(std::move(sock), client_ctx);

                // Launch a background coroutine to handle the client
                co_spawn(
                    exec,
                    handle_client(std::move(client_ssl), target_host, target_port, context),
                    asio::detached
                );
            }
        }
        catch (const boost::system::system_error& e)
        {
            context.logger.error(awl::format() << "runProxy boost::system::system_error: " << e.what());
        }
        catch (const std::exception& e)
        {
            context.logger.error(awl::format() << "runProxy std::exception: " << e.what());
        }
    }
}

// Testing from WSL:
// export ad_ip="172.24.48.1"
// export ad_user="administrator@my.local"
// export ad_password="1234@abc"
//
// export LDAPTLS_REQCERT=never
//
// ldapsearch -H ldaps://$ad_ip:12345 -x -D $ad_user -w $ad_password -b "DC=my,DC=local"

AWL_EXAMPLE(AsioTcpProxy)
{
    AWL_ATTRIBUTE(unsigned int, listen_port, 12345);
    AWL_ATTRIBUTE(std::string, cert_file, "ldap.crt");
    AWL_ATTRIBUTE(std::string, key_file, "ldap.key");
    AWL_ATTRIBUTE(std::string, target, "192.168.0.123:636");

    // SSL context for the client side (proxy acts as a server)
    ssl::context client_ctx(ssl::context::tlsv12_server);

    client_ctx.set_options(
        ssl::context::default_workarounds
        | ssl::context::no_sslv2
        | ssl::context::no_sslv3
        | ssl::context::single_dh_use
    );

    client_ctx.use_certificate_chain_file(cert_file);
    client_ctx.use_private_key_file(key_file, ssl::context::pem);

    auto pos = target.find(':');
    const std::string target_host = target.substr(0, pos);
    const std::string target_port = target.substr(pos + 1);

    asio::thread_pool ioc(1);

    asio::co_spawn(
        ioc,
        runProxy(tcp::endpoint(tcp::v4(), static_cast<unsigned short>(listen_port)), std::move(client_ctx),
            target_host, target_port, context),
        asio::detached);

    ioc.join();
}
