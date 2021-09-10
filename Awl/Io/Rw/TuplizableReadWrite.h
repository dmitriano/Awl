/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Serializable.h"

#include <tuple> 
#include <utility> 
#include <type_traits>

namespace awl::io
{
    //Implementing Read/WriteEach with fold expressions.

    template<class Stream, typename ... Fields, class Context = FakeContext>
    void ReadEach(Stream & s, std::tuple<Fields& ...> val, const Context & ctx = {})
    {
        for_each(val, [&s, &ctx](auto& field) { Read(s, field, ctx); });
    }

    template<class Stream, typename ... Fields, class Context = FakeContext>
    void WriteEach(Stream & s, const std::tuple<Fields& ...> & val, const Context & ctx = {})
    {
        for_each(val, [&s, &ctx](auto& field) { Write(s, field, ctx); });
    }

    //A tuple of references is passed by value.
    template<class Stream, typename ... Fields, class Context = FakeContext>
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
    void Write(Stream & s, const std::tuple<Fields& ...> & val, const Context & ctx = {})
    {
        WriteEach(s, val, ctx);
    }

    template <class Stream, typename T, class Context = FakeContext>
    typename std::enable_if<is_tuplizable_v<T>, void>::type Read(Stream & s, T & val, const Context & ctx = {})
    {
        if constexpr (std::is_same_v<Context, FakeContext>)
        {
            Read(s, object_as_tuple(val), ctx);
        }
        else
        {
            ctx.ReadV(s, val);
        }
    }

    template <class Stream, typename T, class Context = FakeContext>
    typename std::enable_if<is_tuplizable_v<T>, void>::type Write(Stream & s, const T & val, const Context & ctx = {})
    {
        if constexpr (std::is_same_v<Context, FakeContext>)
        {
            Write(s, object_as_tuple(val), ctx);
        }
        else
        {
            ctx.WriteV(s, val);
        }
    }
}
