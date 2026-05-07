#include "Awl/StringFormat.h"
#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/experimental/promise.hpp>
#include <boost/asio/experimental/use_promise.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/multiple_exceptions.hpp>
#include <openssl/ssl.h>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <memory>
#include <mutex>
#include <string>

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = asio::ip::tcp;

namespace
{
    struct SocketOptions
    {
        bool tcp_nodelay;
        bool linger;
        int linger_timeout;
    };

    struct ProxyStats
    {
        std::atomic_uint64_t active_sessions = 0;
        std::atomic_uint64_t active_upstream_handshakes = 0;
        std::atomic_uint64_t refused_total = 0;
        std::atomic_uint64_t refused_interval = 0;
        std::atomic_uint64_t reset_total = 0;
        std::atomic_uint64_t reset_interval = 0;
    };

    class AtomicCounterGuard
    {
    public:
        explicit AtomicCounterGuard(std::atomic_uint64_t& counter) :
            counter_(counter)
        {
            counter_.fetch_add(1, std::memory_order_relaxed);
        }

        ~AtomicCounterGuard()
        {
            counter_.fetch_sub(1, std::memory_order_relaxed);
        }

        AtomicCounterGuard(const AtomicCounterGuard&) = delete;
        AtomicCounterGuard& operator=(const AtomicCounterGuard&) = delete;

    private:
        std::atomic_uint64_t& counter_;
    };

    void set_socket_options(
        tcp::socket& socket,
        const SocketOptions& options,
        const awl::testing::TestContext& context,
        const char* socket_name)
    {
        boost::system::error_code ec;

        socket.set_option(tcp::no_delay(options.tcp_nodelay), ec);
        if (ec)
        {
            context.logger->error("{}: failed to set TCP_NODELAY: {}", socket_name, ec.message());
        }

        if (options.linger)
        {
            socket.set_option(asio::socket_base::linger(true, options.linger_timeout), ec);
            if (ec)
            {
                context.logger->error("{}: failed to set SO_LINGER: {}", socket_name, ec.message());
            }
        }
    }

    void close_socket(tcp::socket& socket)
    {
        boost::system::error_code ignored_ec;
        socket.shutdown(tcp::socket::shutdown_both, ignored_ec);
        socket.close(ignored_ec);
    }

    class TlsClientSessionCache
    {
    public:
        void apply(ssl::stream<tcp::socket>& stream, const awl::testing::TestContext& context)
        {
            auto session = copy();
            if (!session)
            {
                return;
            }

            const int result = SSL_set_session(stream.native_handle(), session.get());
            context.logger->trace("TLS upstream session reuse requested: {}", result != 0);
        }

        void store(ssl::stream<tcp::socket>& stream, const awl::testing::TestContext& context)
        {
            SSL_SESSION* session = SSL_get1_session(stream.native_handle());
            if (session == nullptr)
            {
                return;
            }

            std::lock_guard lock(mutex_);
            session_.reset(session);
            context.logger->trace("TLS upstream session cached. Reused: {}", SSL_session_reused(stream.native_handle()) != 0);
        }

    private:
        struct SslSessionDeleter
        {
            void operator()(SSL_SESSION* session) const noexcept
            {
                SSL_SESSION_free(session);
            }
        };

        using SslSessionPtr = std::unique_ptr<SSL_SESSION, SslSessionDeleter>;

        SslSessionPtr copy()
        {
            std::lock_guard lock(mutex_);
            if (!session_)
            {
                return nullptr;
            }

            if (SSL_SESSION_up_ref(session_.get()) != 1)
            {
                return nullptr;
            }

            return SslSessionPtr(session_.get());
        }

        std::mutex mutex_;
        SslSessionPtr session_;
    };

    std::string describe_exception(std::exception_ptr exception)
    {
        if (!exception)
        {
            return "empty exception";
        }

        try
        {
            std::rethrow_exception(exception);
        }
        catch (const boost::asio::multiple_exceptions& e)
        {
            return std::string(e.what()) + "; first nested exception: " + describe_exception(e.first_exception());
        }
        catch (const std::exception& e)
        {
            return e.what();
        }
        catch (...)
        {
            return "unknown non-standard exception";
        }
    }

    void count_network_exception(const boost::system::error_code& code, const std::shared_ptr<ProxyStats>& stats)
    {
        if (code == asio::error::connection_refused)
        {
            stats->refused_total.fetch_add(1, std::memory_order_relaxed);
            stats->refused_interval.fetch_add(1, std::memory_order_relaxed);
        }
        else if (code == asio::error::connection_reset)
        {
            stats->reset_total.fetch_add(1, std::memory_order_relaxed);
            stats->reset_interval.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void count_exception(std::exception_ptr exception, const std::shared_ptr<ProxyStats>& stats)
    {
        if (!exception)
        {
            return;
        }

        try
        {
            std::rethrow_exception(exception);
        }
        catch (const boost::system::system_error& e)
        {
            count_network_exception(e.code(), stats);
        }
        catch (const boost::asio::multiple_exceptions& e)
        {
            count_exception(e.first_exception(), stats);
        }
        catch (...)
        {
        }
    }

    asio::awaitable<void> log_proxy_stats(
        std::shared_ptr<ProxyStats> stats,
        const awl::testing::TestContext& context)
    {
        auto exec = co_await asio::this_coro::executor;
        asio::steady_timer timer(exec);

        while (true)
        {
            timer.expires_after(std::chrono::seconds(1));
            co_await timer.async_wait(asio::use_awaitable);

            const auto refused = stats->refused_interval.exchange(0, std::memory_order_relaxed);
            const auto reset = stats->reset_interval.exchange(0, std::memory_order_relaxed);

            context.logger->trace(
                "proxy stats: active_sessions={}, active_upstream_handshakes={}, refused/s={}, refused_total={}, reset/s={}, reset_total={}",
                stats->active_sessions.load(std::memory_order_relaxed),
                stats->active_upstream_handshakes.load(std::memory_order_relaxed),
                refused,
                stats->refused_total.load(std::memory_order_relaxed),
                reset,
                stats->reset_total.load(std::memory_order_relaxed));
        }
    }

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
                context.logger->trace("transfer: connection closed (stream truncated)");
            }
            else if (e.code() == boost::asio::error::eof)
            {
                context.logger->trace("transfer: EOF");
            }
            else if (e.code() == asio::error::connection_reset)
            {
                context.logger->trace("transfer: Connection Reset.");
            }
            else if (
                e.code() == boost::system::errc::operation_canceled
                || e.code() == boost::asio::error::operation_aborted)
            {
                context.logger->trace("transfer: Operation cancelled.");
            }
            else if (e.code().category() == boost::system::system_category() && e.code().value() == 10054)
            {
                context.logger->trace("transfer Windows Error: WSAECONNRESET.");
            }
            else if (e.code() == boost::system::errc::broken_pipe)
            {
                // The peer can close while the opposite transfer coroutine is still writing.
                context.logger->trace("transfer: Broken pipe.");
            }
            else
            {
                context.logger->error(_T("transfer boost::system::system_error: {}"), awl::fromACString(e.what()));
            }
        }
        catch (std::exception& e)
        {
            // Any exception = stop proxying
            context.logger->error(_T("transfer std::exception: {}"), awl::fromACString(e.what()));
        }
    }

    asio::awaitable<void> bidirectional_transfer(
        ssl::stream<tcp::socket>& client_ssl,
        ssl::stream<tcp::socket>& server_ssl,
        const awl::testing::TestContext& context)
    {
        using namespace boost::asio::experimental::awaitable_operators;

        co_await(transfer(client_ssl, server_ssl, context) || transfer(server_ssl, client_ssl, context));

        close_socket(client_ssl.next_layer());
        close_socket(server_ssl.next_layer());
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
        std::shared_ptr<ssl::context> server_ctx,
        std::shared_ptr<TlsClientSessionCache> tls_client_session_cache,
        std::shared_ptr<ProxyStats> stats,
        const std::string& target_host,
        const std::string& target_port,
        SocketOptions socket_options,
        const awl::testing::TestContext& context)
    {
        AtomicCounterGuard active_session(stats->active_sessions);

        try
        {
            // Get the current coroutine executor
            auto exec = co_await asio::this_coro::executor;

            // Create an SSL client for the target server
            tcp::resolver resolver(exec);
            auto endpoints = co_await resolver.async_resolve(target_host, target_port, asio::use_awaitable);

            ssl::stream<tcp::socket> server_ssl(exec, *server_ctx);

            auto connect_server = [&]() -> asio::awaitable<void>
            {
                AtomicCounterGuard active_upstream_handshake(stats->active_upstream_handshakes);

                co_await asio::async_connect(server_ssl.next_layer(), endpoints, asio::use_awaitable);
                set_socket_options(server_ssl.next_layer(), socket_options, context, "server socket");
                tls_client_session_cache->apply(server_ssl, context);
                co_await server_ssl.async_handshake(ssl::stream_base::client, asio::use_awaitable);
                tls_client_session_cache->store(server_ssl, context);
            };

            using namespace boost::asio::experimental::awaitable_operators;

            // The proxy can connect to DC while the client-side TLS handshake is in progress.
            co_await(
                client_ssl.async_handshake(ssl::stream_base::server, asio::use_awaitable)
                && connect_server());

            co_await bidirectional_transfer(client_ssl, server_ssl, context);
        }
        catch (const boost::system::system_error& e)
        {
            count_network_exception(e.code(), stats);

            if (e.code() == boost::asio::ssl::error::stream_truncated)
            {
                // Normal termination: SSL shutdown was not sent
                context.logger->trace("handle_client: connection closed (stream truncated)");
            }
            else
            {
                context.logger->error(_T("handle_client exception: {}"), awl::fromACString(e.what()));
            }
        }
        catch (const boost::asio::multiple_exceptions& e)
        {
            count_exception(e.first_exception(), stats);
            const std::string first_exception = describe_exception(e.first_exception());
            context.logger->error(
                _T("handle_client exception: {}; first nested exception: {}"),
                awl::fromACString(e.what()),
                awl::fromACString(first_exception.c_str()));
        }
        catch (const std::exception& e)
        {
            context.logger->error(_T("handle_client exception: {}"), awl::fromACString(e.what()));
        }
    }

    asio::awaitable<void> runProxy(
        tcp::endpoint listen_endpoint,
        std::shared_ptr<ssl::context> client_ctx,
        std::shared_ptr<ssl::context> server_ctx,
        std::shared_ptr<TlsClientSessionCache> tls_client_session_cache,
        std::shared_ptr<ProxyStats> stats,
        const std::string& target_host, const std::string& target_port,
        SocketOptions socket_options,
        const awl::testing::TestContext& context)
    {
        try
        {
            // Get the executor for the acceptor
            auto exec = co_await asio::this_coro::executor;
            tcp::acceptor acceptor(exec, listen_endpoint);

            while (true)
            {
                auto session_exec = asio::make_strand(exec);
                tcp::socket sock(session_exec);

                co_await acceptor.async_accept(sock, asio::use_awaitable);
                set_socket_options(sock, socket_options, context, "client socket");

                ssl::stream<tcp::socket> client_ssl(std::move(sock), *client_ctx);

                // Launch a background coroutine to handle the client
                co_spawn(
                    session_exec,
                    handle_client(
                        std::move(client_ssl),
                        server_ctx,
                        tls_client_session_cache,
                        stats,
                        target_host,
                        target_port,
                        socket_options,
                        context),
                    asio::detached
                );
            }
        }
        catch (const boost::system::system_error& e)
        {
            context.logger->error(_T("runProxy boost::system::system_error: {}"), awl::fromACString(e.what()));
        }
        catch (const std::exception& e)
        {
            context.logger->error(_T("runProxy std::exception: {}"), awl::fromACString(e.what()));
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
    AWL_ATTRIBUTE(unsigned int, thread_count, 1);
    AWL_ATTRIBUTE(bool, tcp_nodelay, true);
    AWL_ATTRIBUTE(bool, linger, true);
    AWL_ATTRIBUTE(int, linger_timeout, 0);

    const SocketOptions socket_options{ tcp_nodelay, linger, linger_timeout };

    // SSL context for the client side (proxy acts as a server)
    auto client_ctx = std::make_shared<ssl::context>(ssl::context::tlsv12_server);

    client_ctx->set_options(
        ssl::context::default_workarounds
        | ssl::context::no_sslv2
        | ssl::context::no_sslv3
        | ssl::context::single_dh_use
    );

    client_ctx->use_certificate_chain_file(cert_file);
    client_ctx->use_private_key_file(key_file, ssl::context::pem);

    const unsigned char session_id_context[] = "awl-asio-tcp-proxy";
    SSL_CTX_set_session_cache_mode(client_ctx->native_handle(), SSL_SESS_CACHE_SERVER);
    SSL_CTX_set_session_id_context(
        client_ctx->native_handle(),
        session_id_context,
        sizeof(session_id_context) - 1);

    auto server_ctx = std::make_shared<ssl::context>(ssl::context::tlsv12_client);
    server_ctx->set_default_verify_paths();
    SSL_CTX_set_session_cache_mode(server_ctx->native_handle(), SSL_SESS_CACHE_CLIENT);
    auto tls_client_session_cache = std::make_shared<TlsClientSessionCache>();
    auto stats = std::make_shared<ProxyStats>();

    auto pos = target.find(':');
    const std::string target_host = target.substr(0, pos);
    const std::string target_port = target.substr(pos + 1);

    asio::thread_pool ioc(thread_count);

    asio::co_spawn(ioc, log_proxy_stats(stats, context), asio::detached);

    asio::co_spawn(
        ioc,
        runProxy(
            tcp::endpoint(tcp::v4(), static_cast<unsigned short>(listen_port)),
            std::move(client_ctx),
            std::move(server_ctx),
            std::move(tls_client_session_cache),
            stats,
            target_host,
            target_port,
            socket_options,
            context),
        asio::detached);

    ioc.join();
}
