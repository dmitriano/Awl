/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/cancellation_type.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>

#include <functional>
#include <memory>
#include <stop_token>

namespace awl
{
    // Bridges a stop_token to Asio terminal cancellation for one co_spawn task.
    // Construct this adapter and start the task on the supplied strand; use the
    // same strand for the coroutine and retain the adapter until completion.
    // Posting the emit lets co_spawn install its cancellation_state first.
    // With Asio's default cancellation checks, no entry stop_requested() check
    // is needed. Explicit polling is still needed for work without co_await or
    // when automatic cancellation checks are disabled.
    class StopToken
    {
    public:

        template <class Executor>
        StopToken(const boost::asio::strand<Executor>& executor,
            const std::stop_token stop_token) :
            _signal(std::make_shared<boost::asio::cancellation_signal>()),
            _stopCallback(stop_token, [executor, weak_signal = std::weak_ptr(_signal)]
            {
                // Never emit inline, even when request_stop() runs on the strand.
                // The coroutine must first be able to install its I/O handler.
                boost::asio::post(executor, [weak_signal]
                {
                    if (std::shared_ptr<boost::asio::cancellation_signal> signal = weak_signal.lock())
                    {
                        signal->emit(boost::asio::cancellation_type::terminal);
                    }
                });
            })
        {}

        StopToken(const StopToken&) = delete;
        StopToken& operator=(const StopToken&) = delete;
        StopToken(StopToken&&) = delete;
        StopToken& operator=(StopToken&&) = delete;

        boost::asio::cancellation_slot slot() const
        {
            return _signal->slot();
        }

    private:

        std::shared_ptr<boost::asio::cancellation_signal> _signal;
        // Unregister the callback before releasing the signal. A queued emit
        // holds only a weak_ptr and safely does nothing after destruction.
        std::stop_callback<std::function<void()>> _stopCallback;
    };
}
