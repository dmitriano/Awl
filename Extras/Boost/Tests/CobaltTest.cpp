/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "Awl/String.h"
#include "Awl/Separator.h"

#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/cobalt.hpp>
#include <iostream>
#include <iomanip>

using namespace awl::testing;

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
namespace cobalt = boost::cobalt;
namespace this_coro = boost::cobalt::this_coro;
using tcp = asio::ip::tcp;

namespace
{
    // Проксирование SSL → SSL
    cobalt::promise<void> proxy_ssl_to_ssl(ssl::stream<tcp::socket>& from,
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
            // Любое исключение = завершение проксирования
            std::cerr << "handle_client exception: " << e.what() << "\n";
        }

        co_return;
    }

    // Обработка одного клиента
    cobalt::task<void> handle_client(
        ssl::stream<tcp::socket> client_ssl,
        ssl::context& client_ctx,
        const std::string& target_host,
        const std::string& target_port
    )
    {
        try
        {
            // TLS-рукопожатие с клиентом (прокси как сервер)
            co_await client_ssl.async_handshake(ssl::stream_base::server, cobalt::use_op);

            // Получаем executor текущей корутины
            auto exec = co_await this_coro::executor;

            // Создаём SSL-клиент для целевого сервера
            tcp::resolver resolver(exec);
            auto endpoints = co_await resolver.async_resolve(target_host, target_port, cobalt::use_op);

            ssl::context server_ctx(ssl::context::tlsv12_client);
            server_ctx.set_default_verify_paths();

            ssl::stream<tcp::socket> server_ssl(exec, server_ctx);
            co_await asio::async_connect(server_ssl.next_layer(), endpoints, cobalt::use_op);
            co_await server_ssl.async_handshake(ssl::stream_base::client, cobalt::use_op);

            // Запуск проксирования в обе стороны параллельно
            co_await cobalt::race(
                proxy_ssl_to_ssl(client_ssl, server_ssl),
                proxy_ssl_to_ssl(server_ssl, client_ssl)
            );
        }
        catch (boost::system::system_error& e)
        {
            if (e.code() == boost::asio::ssl::error::stream_truncated)
            {
                // Нормальное завершение: SSL shutdown не был отправлен
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

        co_return;
    }
}

// Главный цикл
cobalt::main co_main(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cerr << "Usage: ssl_tcp_proxy <listen_port> <cert.pem> <key.pem> <target_host:port>\n";
        co_return 1;
    }

    unsigned short listen_port = static_cast<unsigned short>(std::stoi(argv[1]));
    const char* cert_file = argv[2];
    const char* key_file = argv[3];
    std::string target = argv[4];

    auto pos = target.find(':');
    std::string target_host = target.substr(0, pos);
    std::string target_port = target.substr(pos + 1);

    // SSL-контекст для стороны клиента (прокси как сервер)
    ssl::context client_ctx(ssl::context::tlsv12_server);

    client_ctx.set_options(
        ssl::context::default_workarounds
        | ssl::context::no_sslv2
        | ssl::context::no_sslv3
        | ssl::context::single_dh_use
    );

    client_ctx.use_certificate_chain_file(cert_file);
    client_ctx.use_private_key_file(key_file, ssl::context::pem);

    // Получаем executor для acceptor
    auto exec = co_await this_coro::executor;
    tcp::acceptor acceptor(exec, tcp::endpoint(tcp::v4(), listen_port));

    while (true)
    {
        tcp::socket sock = co_await acceptor.async_accept(cobalt::use_op);
        ssl::stream<tcp::socket> client_ssl(std::move(sock), client_ctx);

        // Запускаем корутину-обработчик клиента в фоне
        cobalt::spawn(
            exec,
            handle_client(std::move(client_ssl), client_ctx, target_host, target_port),
            asio::detached
        );
    }

    co_return 0;
}
