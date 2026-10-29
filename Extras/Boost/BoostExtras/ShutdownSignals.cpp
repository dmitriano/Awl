#include "BoostExtras/ShutdownSignals.h"

#include "Awl/Exception.h"

#include <boost/asio/use_awaitable.hpp>

#include <csignal>
#include <format>

namespace awl
{
    ShutdownSignals::ShutdownSignals(const boost::asio::any_io_executor executor) :
        _signals(executor, SIGINT, SIGTERM)
    {
#ifdef _WIN32
        _signals.add(SIGBREAK);
#endif
    }

    boost::asio::awaitable<ShutdownSignal> ShutdownSignals::asyncWait()
    {
        const int signal_number = co_await _signals.async_wait(boost::asio::use_awaitable);

        switch (signal_number)
        {
        case SIGINT:
            co_return ShutdownSignal::Interrupt;
        case SIGTERM:
            co_return ShutdownSignal::Terminate;
#ifdef _WIN32
        case SIGBREAK:
            co_return ShutdownSignal::Break;
#endif
        default:
            throw awl::GeneralException(std::format("Unexpected shutdown signal: {}.", signal_number));
        }
    }

    void ShutdownSignals::cancel()
    {
        _signals.cancel();
    }
}
