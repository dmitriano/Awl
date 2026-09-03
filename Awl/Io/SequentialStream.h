/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <concepts>

namespace awl::io
{
    class SequentialInputStream
    {
    public:

        virtual bool end() = 0;

        virtual size_t read(std::byte* buffer, size_t count) = 0;

        virtual ~SequentialInputStream() = default;
    };

    class SequentialOutputStream
    {
    public:

        virtual void write(const std::byte* buffer, size_t count) = 0;

        virtual ~SequentialOutputStream() = default;
    };

    template <class T>
    concept sequential_input_stream = requires(T& t)
    {
        { t.end() } -> std::same_as<bool>;
        { t.read(std::declval<std::byte*>(), std::declval<size_t>()) } -> std::convertible_to<size_t>;
    };

    template <class T>
    concept sequential_output_stream = requires(T& t)
    {
        { t.write(std::declval<const std::byte*>(), std::declval<size_t>()) } -> std::same_as<void>;
    };

    static_assert(sequential_input_stream<SequentialInputStream>);
    static_assert(sequential_output_stream<SequentialOutputStream>);
}
