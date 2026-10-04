#pragma once

#include <boost/asio/experimental/concurrent_channel.hpp>

namespace awl
{
    // Thread-safe Asio channel. Supply its executor and capacity at construction;
    // await sends to apply backpressure when the channel is full.
    template <typename ExecutorOrSignature, typename... Signatures>
    using Channel = boost::asio::experimental::concurrent_channel<
        ExecutorOrSignature, Signatures...>;
}
