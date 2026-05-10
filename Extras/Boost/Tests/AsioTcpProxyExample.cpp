#include "Awl/StringFormat.h"
#include "Awl/Testing/UnitTest.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/experimental/promise.hpp>
#include <boost/asio/experimental/use_promise.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
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

    constexpr std::chrono::milliseconds TLS_SHUTDOWN_TIMEOUT{ 1000 };

    struct ProxyStats
    {
        std::atomic_uint64_t active_sessions = 0;
        std::atomic_uint64_t upstream_waiters = 0;
        std::atomic_uint64_t active_upstream_handshakes = 0;
        std::atomic_uint64_t refused_total = 0;
        std::atomic_uint64_t refused_interval = 0;
        std::atomic_uint64_t reset_total = 0;
        std::atomic_uint64_t reset_interval = 0;
        std::atomic_uint64_t rejected_by_proxy_total = 0;
        std::atomic_uint64_t rejected_by_proxy_interval = 0;
        std::atomic_uint64_t upstream_timeout_total = 0;
        std::atomic_uint64_t upstream_timeout_interval = 0;
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

    class UpstreamQueueFullException : public std::exception
    {
    public:

        const char* what() const noexcept override
        {
            return "upstream queue is full";
        }
    };

    class UpstreamHandshakeTimeoutException : public std::exception
    {
    public:

        const char* what() const noexcept override
        {
            return "upstream connect/handshake timeout";
        }
    };

    bool try_reserve_upstream_waiter(
        const std::shared_ptr<ProxyStats>& stats,
        unsigned int max_upstream_waiters)
    {
        if (max_upstream_waiters == 0)
        {
            stats->upstream_waiters.fetch_add(1, std::memory_order_relaxed);
            return true;
        }

        auto current = stats->upstream_waiters.load(std::memory_order_relaxed);
        while (current < max_upstream_waiters)
        {
            if (stats->upstream_waiters.compare_exchange_weak(
                current,
                current + 1,
                std::memory_order_relaxed,
                std::memory_order_relaxed))
            {
                return true;
            }
        }

        return false;
    }

    void count_proxy_rejection(const std::shared_ptr<ProxyStats>& stats)
    {
        stats->rejected_by_proxy_total.fetch_add(1, std::memory_order_relaxed);
        stats->rejected_by_proxy_interval.fetch_add(1, std::memory_order_relaxed);
    }

    void count_upstream_timeout(const std::shared_ptr<ProxyStats>& stats)
    {
        stats->upstream_timeout_total.fetch_add(1, std::memory_order_relaxed);
        stats->upstream_timeout_interval.fetch_add(1, std::memory_order_relaxed);
    }

    class UpstreamConcurrencyLimiter
    {
    public:

        UpstreamConcurrencyLimiter(asio::any_io_executor executor, unsigned int limit) :
            limit_(limit),
            channel_(std::move(executor), limit == 0 ? 1 : limit)
        {
            for (unsigned int i = 0; i < limit_; ++i)
            {
                channel_.try_send(boost::system::error_code{});
            }
        }

        bool enabled() const
        {
            return limit_ != 0;
        }

        asio::awaitable<void> acquire()
        {
            if (!enabled())
            {
                co_return;
            }

            co_await channel_.async_receive(asio::use_awaitable);
        }

        void release()
        {
            if (enabled())
            {
                [[maybe_unused]] const bool sent = channel_.try_send(boost::system::error_code{});
            }
        }

    private:

        unsigned int limit_;
        boost::asio::experimental::concurrent_channel<void(boost::system::error_code)> channel_;
    };

    class UpstreamConcurrencyPermit
    {
    public:

        explicit UpstreamConcurrencyPermit(std::shared_ptr<UpstreamConcurrencyLimiter> limiter) :
            limiter_(std::move(limiter))
        {}

        ~UpstreamConcurrencyPermit()
        {
            release();
        }

        UpstreamConcurrencyPermit(const UpstreamConcurrencyPermit&) = delete;
        UpstreamConcurrencyPermit& operator=(const UpstreamConcurrencyPermit&) = delete;

        UpstreamConcurrencyPermit(UpstreamConcurrencyPermit&& other) noexcept :
            limiter_(std::move(other.limiter_))
        {}

        UpstreamConcurrencyPermit& operator=(UpstreamConcurrencyPermit&& other) noexcept
        {
            if (this != &other)
            {
                release();
                limiter_ = std::move(other.limiter_);
            }

            return *this;
        }

    private:

        void release()
        {
            if (limiter_)
            {
                limiter_->release();
                limiter_.reset();
            }
        }

        std::shared_ptr<UpstreamConcurrencyLimiter> limiter_;
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

    bool is_expected_tls_shutdown_error(const boost::system::error_code& code)
    {
        return code == boost::asio::ssl::error::stream_truncated
            || code == boost::asio::error::eof
            || code == boost::asio::error::connection_reset
            || code == boost::asio::error::operation_aborted
            || code == boost::system::errc::operation_canceled
            || code == boost::system::errc::broken_pipe
            || (code.category() == boost::system::system_category() && code.value() == 10054);
    }

    asio::awaitable<void> async_tls_shutdown(
        ssl::stream<tcp::socket>& stream,
        const char* stream_name,
        const awl::testing::TestContext& context)
    {
        if (!stream.next_layer().is_open())
        {
            co_return;
        }

        asio::steady_timer timer(stream.get_executor());
        timer.expires_after(TLS_SHUTDOWN_TIMEOUT);

        using namespace boost::asio::experimental::awaitable_operators;

        boost::system::error_code shutdown_ec;
        boost::system::error_code timer_ec;

        co_await(
            stream.async_shutdown(asio::redirect_error(asio::use_awaitable, shutdown_ec))
            || timer.async_wait(asio::redirect_error(asio::use_awaitable, timer_ec)));

        if (!timer_ec)
        {
            context.logger->trace(
                "{} TLS shutdown timed out after {} ms",
                stream_name,
                TLS_SHUTDOWN_TIMEOUT.count());
        }

        if (shutdown_ec)
        {
            if (is_expected_tls_shutdown_error(shutdown_ec))
            {
                context.logger->trace(
                    "{} TLS shutdown finished with an expected error: {}",
                    stream_name,
                    shutdown_ec.message());
            }
            else
            {
                context.logger->error(
                    "{} TLS shutdown failed: {}",
                    stream_name,
                    shutdown_ec.message());
            }
        }
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

    bool is_upstream_queue_full_exception(std::exception_ptr exception)
    {
        if (!exception)
        {
            return false;
        }

        try
        {
            std::rethrow_exception(exception);
        }
        catch (const UpstreamQueueFullException&)
        {
            return true;
        }
        catch (const boost::asio::multiple_exceptions& e)
        {
            return is_upstream_queue_full_exception(e.first_exception());
        }
        catch (...)
        {
            return false;
        }
    }

    bool is_operation_canceled_exception(std::exception_ptr exception)
    {
        if (!exception)
        {
            return false;
        }

        try
        {
            std::rethrow_exception(exception);
        }
        catch (const boost::system::system_error& e)
        {
            return
                e.code() == boost::system::errc::operation_canceled
                || e.code() == boost::asio::error::operation_aborted;
        }
        catch (const boost::asio::multiple_exceptions& e)
        {
            return is_operation_canceled_exception(e.first_exception());
        }
        catch (...)
        {
            return false;
        }
    }

    bool is_upstream_handshake_timeout_exception(std::exception_ptr exception)
    {
        if (!exception)
        {
            return false;
        }

        try
        {
            std::rethrow_exception(exception);
        }
        catch (const UpstreamHandshakeTimeoutException&)
        {
            return true;
        }
        catch (const boost::asio::multiple_exceptions& e)
        {
            return is_upstream_handshake_timeout_exception(e.first_exception());
        }
        catch (...)
        {
            return false;
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
            const auto rejected_by_proxy =
                stats->rejected_by_proxy_interval.exchange(0, std::memory_order_relaxed);
            const auto upstream_timeout =
                stats->upstream_timeout_interval.exchange(0, std::memory_order_relaxed);

            context.logger->info(
                "proxy stats: active_sessions={}, upstream_waiters={}, active_upstream_handshakes={}, refused/s={}, refused_total={}, reset/s={}, reset_total={}, rejected_by_proxy/s={}, rejected_by_proxy_total={}, upstream_timeout/s={}, upstream_timeout_total={}",
                stats->active_sessions.load(std::memory_order_relaxed),
                stats->upstream_waiters.load(std::memory_order_relaxed),
                stats->active_upstream_handshakes.load(std::memory_order_relaxed),
                refused,
                stats->refused_total.load(std::memory_order_relaxed),
                reset,
                stats->reset_total.load(std::memory_order_relaxed),
                rejected_by_proxy,
                stats->rejected_by_proxy_total.load(std::memory_order_relaxed),
                upstream_timeout,
                stats->upstream_timeout_total.load(std::memory_order_relaxed));
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

        co_await(
            async_tls_shutdown(client_ssl, "client", context)
            && async_tls_shutdown(server_ssl, "server", context));

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
        std::shared_ptr<UpstreamConcurrencyLimiter> upstream_concurrency_limiter,
        std::shared_ptr<ProxyStats> stats,
        const std::string& target_host,
        const std::string& target_port,
        unsigned int max_upstream_waiters,
        unsigned int upstream_handshake_timeout_ms,
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
                if (!try_reserve_upstream_waiter(stats, max_upstream_waiters))
                {
                    count_proxy_rejection(stats);
                    context.logger->trace(
                        "handle_client: upstream queue is full, closing client. upstream_waiters={}, max_upstream_waiters={}",
                        stats->upstream_waiters.load(std::memory_order_relaxed),
                        max_upstream_waiters);
                    close_socket(client_ssl.next_layer());
                    throw UpstreamQueueFullException();
                }

                {
                    struct ReservedWaiterGuard
                    {
                        explicit ReservedWaiterGuard(std::atomic_uint64_t& counter) :
                            counter_(counter)
                        {}

                        ~ReservedWaiterGuard()
                        {
                            counter_.fetch_sub(1, std::memory_order_relaxed);
                        }

                        ReservedWaiterGuard(const ReservedWaiterGuard&) = delete;
                        ReservedWaiterGuard& operator=(const ReservedWaiterGuard&) = delete;

                        std::atomic_uint64_t& counter_;
                    } upstream_waiter(stats->upstream_waiters);

                    co_await upstream_concurrency_limiter->acquire();
                }

                UpstreamConcurrencyPermit upstream_permit(upstream_concurrency_limiter);
                AtomicCounterGuard active_upstream_handshake(stats->active_upstream_handshakes);

                std::shared_ptr<asio::steady_timer> upstream_timeout_timer;
                std::shared_ptr<asio::cancellation_signal> upstream_cancellation_signal;
                auto upstream_timed_out = std::make_shared<std::atomic_bool>(false);

                if (upstream_handshake_timeout_ms != 0)
                {
                    auto exec = co_await asio::this_coro::executor;
                    upstream_timeout_timer = std::make_shared<asio::steady_timer>(exec);
                    upstream_cancellation_signal = std::make_shared<asio::cancellation_signal>();
                    upstream_timeout_timer->expires_after(std::chrono::milliseconds(upstream_handshake_timeout_ms));
                    upstream_timeout_timer->async_wait(
                        [
                            upstream_timeout_timer,
                            upstream_cancellation_signal,
                            upstream_timed_out,
                            &server_ssl,
                            &context,
                            upstream_handshake_timeout_ms
                        ](const boost::system::error_code& ec)
                        {
                            if (ec)
                            {
                                return;
                            }

                            upstream_timed_out->store(true, std::memory_order_relaxed);
                            context.logger->trace(
                                "upstream connect/handshake timeout after {} ms",
                                upstream_handshake_timeout_ms);
                            upstream_cancellation_signal->emit(asio::cancellation_type::all);
                            close_socket(server_ssl.next_layer());
                        });
                }

                try
                {
                    if (upstream_cancellation_signal)
                    {
                        co_await asio::async_connect(
                            server_ssl.next_layer(),
                            endpoints,
                            asio::bind_cancellation_slot(
                                upstream_cancellation_signal->slot(),
                                asio::use_awaitable));
                    }
                    else
                    {
                        co_await asio::async_connect(server_ssl.next_layer(), endpoints, asio::use_awaitable);
                    }

                    set_socket_options(server_ssl.next_layer(), socket_options, context, "server socket");
                    tls_client_session_cache->apply(server_ssl, context);

                    if (upstream_cancellation_signal)
                    {
                        co_await server_ssl.async_handshake(
                            ssl::stream_base::client,
                            asio::bind_cancellation_slot(
                                upstream_cancellation_signal->slot(),
                                asio::use_awaitable));
                    }
                    else
                    {
                        co_await server_ssl.async_handshake(ssl::stream_base::client, asio::use_awaitable);
                    }

                    tls_client_session_cache->store(server_ssl, context);
                }
                catch (...)
                {
                    if (upstream_timeout_timer)
                    {
                        upstream_timeout_timer->cancel();
                    }

                    if (upstream_timed_out->load(std::memory_order_relaxed))
                    {
                        count_upstream_timeout(stats);
                        throw UpstreamHandshakeTimeoutException();
                    }

                    throw;
                }

                if (upstream_timeout_timer)
                {
                    upstream_timeout_timer->cancel();
                }
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
            else if (
                e.code() == boost::system::errc::operation_canceled
                || e.code() == boost::asio::error::operation_aborted)
            {
                context.logger->trace("handle_client: Operation cancelled.");
            }
            else
            {
                context.logger->error(_T("handle_client exception: {}"), awl::fromACString(e.what()));
            }
        }
        catch (const UpstreamQueueFullException&)
        {
            context.logger->trace("handle_client: upstream queue is full");
        }
        catch (const UpstreamHandshakeTimeoutException&)
        {
            context.logger->trace("handle_client: upstream connect/handshake timeout");
        }
        catch (const boost::asio::multiple_exceptions& e)
        {
            count_exception(e.first_exception(), stats);
            const std::string first_exception = describe_exception(e.first_exception());
            if (
                is_upstream_queue_full_exception(e.first_exception())
                || is_upstream_handshake_timeout_exception(e.first_exception())
                || is_operation_canceled_exception(e.first_exception()))
            {
                context.logger->trace(
                    _T("handle_client exception: {}; first nested exception: {}"),
                    awl::fromACString(e.what()),
                    awl::fromACString(first_exception.c_str()));
            }
            else
            {
                context.logger->error(
                    _T("handle_client exception: {}; first nested exception: {}"),
                    awl::fromACString(e.what()),
                    awl::fromACString(first_exception.c_str()));
            }
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
        std::shared_ptr<UpstreamConcurrencyLimiter> upstream_concurrency_limiter,
        std::shared_ptr<ProxyStats> stats,
        const std::string& target_host, const std::string& target_port,
        unsigned int max_upstream_waiters,
        unsigned int upstream_handshake_timeout_ms,
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
                        upstream_concurrency_limiter,
                        stats,
                        target_host,
                        target_port,
                        max_upstream_waiters,
                        upstream_handshake_timeout_ms,
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
    AWL_ATTRIBUTE(unsigned int, thread_count, 4);
    AWL_ATTRIBUTE(bool, tcp_nodelay, true);
    AWL_ATTRIBUTE(bool, linger, true);
    AWL_ATTRIBUTE(int, linger_timeout, 0);
    AWL_ATTRIBUTE(unsigned int, upstream_concurrency_limit, 200);
    AWL_ATTRIBUTE(unsigned int, max_upstream_waiters, 50);
    AWL_ATTRIBUTE(unsigned int, upstream_handshake_timeout_ms, 3000);

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
    auto upstream_concurrency_limiter =
        std::make_shared<UpstreamConcurrencyLimiter>(ioc.get_executor(), upstream_concurrency_limit);

    asio::co_spawn(ioc, log_proxy_stats(stats, context), asio::detached);

    asio::co_spawn(
        ioc,
        runProxy(
            tcp::endpoint(tcp::v4(), static_cast<unsigned short>(listen_port)),
            std::move(client_ctx),
            std::move(server_ctx),
            std::move(tls_client_session_cache),
            std::move(upstream_concurrency_limiter),
            stats,
            target_host,
            target_port,
            max_upstream_waiters,
            upstream_handshake_timeout_ms,
            socket_options,
            context),
        asio::detached);

    ioc.join();
}
