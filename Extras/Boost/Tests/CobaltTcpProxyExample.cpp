#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/cobalt.hpp>
#include <iostream>
#include <iomanip>

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
namespace cobalt = boost::cobalt;
namespace this_coro = boost::cobalt::this_coro;
using tcp = asio::ip::tcp;

namespace
{
    // Proxying SSL → SSL
    cobalt::promise<void> transfer(ssl::stream<tcp::socket>& from,
        ssl::stream<tcp::socket>& to)
    {
        try
        {
            char buf[8192];
            while (true)
            {
                std::size_t n = co_await from.async_read_some(asio::buffer(buf), cobalt::use_op);
                co_await asio::async_write(to, asio::buffer(buf, n), cobalt::use_op);
            }
        }
        catch (std::exception& e)
        {
            // Any exception = stop proxying
            std::cerr << "handle_client exception: " << e.what() << "\n";
        }
    }

    // Handling a single client
    cobalt::task<void> handle_client(
        ssl::stream<tcp::socket> client_ssl,
        ssl::context& client_ctx,
        const std::string& target_host,
        const std::string& target_port
    )
    {
        try
        {
            // TLS handshake with the client (proxy acts as a server)
            co_await client_ssl.async_handshake(ssl::stream_base::server, cobalt::use_op);

            // Get the current coroutine executor
            auto exec = co_await this_coro::executor;

            // Create an SSL client for the target server
            tcp::resolver resolver(exec);
            auto endpoints = co_await resolver.async_resolve(target_host, target_port, cobalt::use_op);

            ssl::context server_ctx(ssl::context::tlsv12_client);
            server_ctx.set_default_verify_paths();

            ssl::stream<tcp::socket> server_ssl(exec, server_ctx);
            co_await asio::async_connect(server_ssl.next_layer(), endpoints, cobalt::use_op);
            co_await server_ssl.async_handshake(ssl::stream_base::client, cobalt::use_op);

            // Start proxying in both directions in parallel
            co_await cobalt::race(
                transfer(client_ssl, server_ssl),
                transfer(server_ssl, client_ssl)
            );
        }
        catch (boost::system::system_error& e)
        {
            if (e.code() == boost::asio::ssl::error::stream_truncated)
            {
                // Normal termination: SSL shutdown was not sent
                std::cerr << "handle_client: connection closed (stream truncated)\n";
            }
            else
            {
                std::cerr << "handle_client exception: " << e.what() << "\n";
            }
        }
        catch (std::exception& e)
        {
            std::cerr << "handle_client exception: " << e.what() << "\n";
        }
    }

    cobalt::task<void> runProxy(tcp::endpoint listen_endpoint, ssl::context client_ctx,
        const std::string& target_host, const std::string& target_port)
    {
        // Get the executor for the acceptor
        auto exec = co_await this_coro::executor;
        tcp::acceptor acceptor(exec, listen_endpoint);

        while (true)
        {
            tcp::socket sock = co_await acceptor.async_accept(cobalt::use_op);
            ssl::stream<tcp::socket> client_ssl(std::move(sock), client_ctx);

            // Launch a background coroutine to handle the client
            cobalt::spawn(
                exec,
                handle_client(std::move(client_ssl), client_ctx, target_host, target_port),
                asio::detached
            );
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
// ldapsearch -H ldaps://$ad_ip:12345 -x -D $ad_user -w $ad_password -b "DC=my,DC=local" \
//   -s sub -a always -z 1000 "(objectClass=user)" "serviceClassName" "serviceDNSName" "objectClass"

AWL_EXAMPLE(CobaltTcpProxy)
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
        cobalt::run(runProxy(tcp::endpoint(tcp::v4(), static_cast<unsigned short>(listen_port)), std::move(client_ctx),
            target_host, target_port));
    }
    catch (boost::system::system_error& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
