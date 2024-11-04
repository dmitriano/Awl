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
    void ReadEach(Stream & s, std::tuple<Fields& ...> val, const Context & ctx = {})
    {
        for_each(val, [&s, &ctx](auto& field) { Read(s, field, ctx); });
    }

    template<class Stream, typename ... Fields, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void WriteEach(Stream & s, const std::tuple<Fields& ...> & val, const Context & ctx = {})
    {
        for_each(val, [&s, &ctx](auto& field) { Write(s, field, ctx); });
    }

    //A tuple of references is passed by value.
    template<class Stream, typename ... Fields, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void Read(Stream & s, std::tuple<Fields& ...> val, const Context & ctx = {})
    {
        ReadEach(s, val, ctx);
    }

    //A tuple of values is passed by reference. Cannot figure out why this does not compile with VC2017.
    //template<class Stream, typename ... Fields>
    //void Read(Stream & s, std::tuple<Fields ...> & val)
    //{
    //    ReadEach(s, val);
    //}

    template<class Stream, typename ... Fields, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void Write(Stream & s, const std::tuple<Fields& ...> & val, const Context & ctx = {})
    {
        WriteEach(s, val, ctx);
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && is_tuplizable_v<T>)
    void Read(Stream & s, T & val, const Context & ctx = {})
    {
        if constexpr (vts_read_context<Context, Stream, T>)
        {
            ctx.ReadV(s, val);
        }
        else
        {
            Read(s, object_as_tuple(val), ctx);
        }
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && is_tuplizable_v<T>)
    void Write(Stream & s, const T & val, const Context & ctx = {})
    {
        if constexpr (vts_write_context<Context, Stream, T>)
        {
            ctx.WriteV(s, val);
        }
        else
        {
            Write(s, object_as_tuple(val), ctx);
        }
    }
}
