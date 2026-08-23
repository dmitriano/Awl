/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Tuplizable.h"
#include "Awl/Io/SequentialStream.h"

#include <tuple> 
#include <utility> 
#include <type_traits>

namespace awl::io
{
    //Implementing Read/WriteEach with fold expressions.

    template<class Stream, typename ... Fields, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void readEach(Stream & s, std::tuple<Fields& ...> val, const Context & ctx = {})
    {
        for_each(val, [&s, &ctx](auto& field) { read(s, field, ctx); });
    }

    template<class Stream, typename ... Fields, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void writeEach(Stream & s, const std::tuple<Fields& ...> & val, const Context & ctx = {})
    {
        for_each(val, [&s, &ctx](auto& field) { write(s, field, ctx); });
    }

    //A tuple of references is passed by value.
    template<class Stream, typename ... Fields, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream & s, std::tuple<Fields& ...> val, const Context & ctx = {})
    {
        readEach(s, val, ctx);
    }

    //A tuple of values is passed by reference. Cannot figure out why this does not compile with VC2017.
    //template<class Stream, typename ... Fields>
    //void read(Stream & s, std::tuple<Fields ...> & val)
    //{
    //    readEach(s, val);
    //}

    template<class Stream, typename ... Fields, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream & s, const std::tuple<Fields& ...> & val, const Context & ctx = {})
    {
        writeEach(s, val, ctx);
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && tuplizable<T>)
    void read(Stream & s, T & val, const Context & ctx = {})
    {
        if constexpr (vts_read_context<Context, Stream, T>)
        {
            ctx.readV(s, val);
        }
        else
        {
            read(s, object_as_tuple(val), ctx);
        }
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && tuplizable<T>)
    void write(Stream & s, const T & val, const Context & ctx = {})
    {
        if constexpr (vts_write_context<Context, Stream, T>)
        {
            ctx.writeV(s, val);
        }
        else
        {
            write(s, object_as_tuple(val), ctx);
        }
    }
}
