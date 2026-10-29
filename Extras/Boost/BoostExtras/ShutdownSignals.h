#pragma once

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/signal_set.hpp>

namespace awl
{
    enum class ShutdownSignal
    {
        Interrupt,
        Terminate,
        Break
    };

    // Register process shutdown signals on a caller-owned strand executor.
    // Wait, cancel and destruction require that same strand (or an idle context).
    // The object must outlive its wait coroutines. Cancel AND join waits before
    // stopping the executor. No threads, pools or strands are created here.
    class ShutdownSignals
    {
    public:

        explicit ShutdownSignals(boost::asio::any_io_executor executor);

        ShutdownSignals(const ShutdownSignals&) = delete;

        ShutdownSignals& operator = (const ShutdownSignals&) = delete;

        // SIGINT/SIGTERM on all platforms, plus SIGBREAK on Windows.
        // Errors, including cancellation, propagate as Asio system_error.
        boost::asio::awaitable<ShutdownSignal> asyncWait();

        // Cancel active waits; a later asyncWait() can be started again.
        void cancel();

    private:

        boost::asio::signal_set _signals;
    };
}
