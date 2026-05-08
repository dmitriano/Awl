/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <utility>

#pragma once

namespace awl::io
{
    template <class T>
    concept limited_context = requires(T& t)
    {
        { std::as_const(t).max_length() } -> std::convertible_to<size_t>;
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

        LimitedContext(size_t len) : _len(len) {}

        size_t max_length() const
        {
            return _len;
        }

    private:

        size_t _len;
    };

    static_assert(limited_context<LimitedContext>);
}
