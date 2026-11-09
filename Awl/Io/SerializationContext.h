/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/IoException.h"

#include <concepts>
#include <cstddef>
#include <utility>

namespace awl::io
{
    template <class T>
    concept limited_context = requires(const T& ctx, size_t byte_count)
    {
        { ctx.consume(byte_count) } -> std::same_as<void>;
    };

    template <class T, class Stream, typename Val>
    concept vts_read_context = requires(T& t)
    {
        { std::as_const(t).template readV<Val>(std::declval<Stream&>(), std::declval<Val&>()) } -> std::same_as<void>;
    };

    template <class T, class Stream, typename Val>
    concept vts_write_context = requires(T& t)
    {
        { std::as_const(t).writeV(std::declval<Stream&>(), std::declval<const Val&>()) } -> std::same_as<void>;
    };

    class FakeContext {};

    class LimitedContext
    {
    public:

        explicit LimitedContext(size_t limit) : _limit(limit) {}

        // Budget units include requested bytes and estimated storage created by resize.
        // Failed reads do not refund the budget; use a new context for a retry.
        void consume(size_t byte_count) const
        {
            if (byte_count > _limit - _consumed)
            {
                throw IoError(std::format(_T("Deserialization budget exceeded. Limit: {}, consumed: {}, requested: {}."),
                    _limit, _consumed, byte_count));
            }

            _consumed += byte_count;
        }

    private:

        const size_t _limit;
        mutable size_t _consumed = 0;
    };

    static_assert(limited_context<LimitedContext>);
}
