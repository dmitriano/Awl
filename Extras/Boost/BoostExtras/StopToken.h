/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/cancellation_type.hpp>
#include <boost/asio/post.hpp>

#include <functional>
#include <memory>
#include <stop_token>

namespace awl
{
    // Bridges a stop_token to Asio terminal cancellation for one co_spawn task.
    // The executor must wrap a strand. Construct this adapter and immediately
    // co_spawn the task on that same strand; retain the adapter until completion.
    // Posting the emit lets co_spawn install its cancellation_state first.
    // With Asio's default cancellation checks, no entry stop_requested() check
    // is needed. Explicit polling is still needed for work without co_await or
    // when automatic cancellation checks are disabled.
    class StopToken
    {
    public:

        StopToken(const boost::asio::any_io_executor& executor,
            const std::stop_token stop_token) :
            _signal(std::make_shared<boost::asio::cancellation_signal>()),
            _stopCallback(stop_token, [executor, weak_signal = std::weak_ptr(_signal)]
            {
                // Never emit inline, even when request_stop() runs on the strand.
                // co_spawn must first install its cancellation_state handler.
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
