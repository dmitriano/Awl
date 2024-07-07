/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <concepts>

namespace awl::io
{
    class SequentialInputStream
    {
    public:

        virtual bool End() = 0;

        virtual size_t Read(uint8_t* buffer, size_t count) = 0;

        virtual ~SequentialInputStream() = default;
    };

    class SequentialOutputStream
    {
    public:

        virtual void Write(const uint8_t* buffer, size_t count) = 0;

        virtual ~SequentialOutputStream() = default;
    };

    template <class T>
    concept sequential_input_stream = requires(T t)
    {
        { t.End() } -> std::same_as<bool>;
        { t.Read(std::declval<uint8_t*>(), std::declval<size_t>()) } -> std::convertible_to<size_t>;
    };

    template <class T>
    concept sequential_output_stream = requires(T t)
    {
        { t.Write(std::declval<const uint8_t*>(), std::declval<size_t>()) } -> std::same_as<void>;
    };

    static_assert(sequential_input_stream<SequentialInputStream>);
    static_assert(sequential_output_stream<SequentialOutputStream>);
}
