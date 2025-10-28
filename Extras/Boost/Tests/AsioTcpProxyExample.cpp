#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/experimental/promise.hpp>
#include <boost/asio/experimental/use_promise.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <iostream>
#include <iomanip>

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = asio::ip::tcp;
using namespace boost::asio::experimental::awaitable_operators;

namespace
{
    // Proxying SSL → SSL
    asio::awaitable<void> transfer(ssl::stream<tcp::socket>& from,
        ssl::stream<tcp::socket>& to)
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
        catch (boost::system::system_error& e)
        {
            if (e.code() == boost::asio::ssl::error::stream_truncated)
            {
                // Normal termination: SSL shutdown was not sent
                std::cerr << "handle_client: connection closed (stream truncated)\n";
            }
            if (e.code() == boost::asio::error::eof)
            {
                std::cerr << "handle_client: EOF" << std::endl;
            }
            if (e.code() == boost::system::errc::operation_canceled || e.code() == boost::asio::error::operation_aborted)
            {
                std::cerr << "handle_client: Operation cancelled." << std::endl;
            }
            else
            {
                std::cerr << "handle_client boost::system::system_error: " << e.what() << std::endl;
            }
        }
        catch (std::exception& e)
        {
            // Any exception = stop proxying
            std::cerr << "handle_client std::exception: " << e.what() << std::endl;
        }
    }

    // Handling a single client
    asio::awaitable<void> handle_client(
        ssl::stream<tcp::socket> client_ssl,
        ssl::context& client_ctx,
        const std::string& target_host,
        const std::string& target_port
    )
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

            co_await (transfer(client_ssl, server_ssl) || transfer(server_ssl, client_ssl));
        }
        catch (boost::system::system_error& e)
        {
            if (e.code() == boost::asio::ssl::error::stream_truncated)
            {
                // Normal termination: SSL shutdown was not sent
                std::cerr << "handle_client: connection closed (stream truncated)" << std::endl;
            }
            else
            {
                std::cerr << "handle_client exception: " << e.what() << std::endl;
            }
        }
        catch (std::exception& e)
        {
            std::cerr << "handle_client exception: " << e.what() << std::endl;
        }
    }

    asio::awaitable<void> runProxy(tcp::endpoint listen_endpoint, ssl::context client_ctx,
        const std::string& target_host, const std::string& target_port)
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
                handle_client(std::move(client_ssl), client_ctx, target_host, target_port),
                asio::detached
            );
        }
    }
}

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

    try
    {
        asio::thread_pool ioc(1);

        co_spawn(
            ioc,
            runProxy(tcp::endpoint(tcp::v4(), static_cast<unsigned short>(listen_port)), std::move(client_ctx),
                target_host, target_port),
            asio::detached);

        ioc.join();
    }
    catch (boost::system::system_error& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
