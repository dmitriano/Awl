#pragma once

#include "Awl/Io/Rw/ReadRaw.h"
#include "Awl/Io/Rw/VectorReadWrite.h"

#include "Awl/BitMap.h"
#include "Awl/Io/Rw/RwAdapters.h"

#include <bitset>
#include <type_traits>

namespace awl::io
{
    //Looks like std::bitset<N> does not have value_type, so we use BitSetAdapter
    template <class Stream, std::size_t N, class Context = FakeContext>
    inline void Read(Stream & s, std::bitset<N> & v, const Context & ctx = {})
    {
        adapters::BitSetAdapter<std::bitset<N>> a(v);
            
        ReadVector(s, a, ctx);
    }

    template <class Stream, std::size_t N, class Context = FakeContext>
    inline void Write(Stream & s, const std::bitset<N> & v, const Context & ctx = {})
    {
        const adapters::BitSetAdapter<const std::bitset<N>> a(v);

        WriteVector(s, a, ctx);
    }

    template <class Stream, typename Enum, typename std::underlying_type<Enum>::type N, class Context = FakeContext>
    inline void Read(Stream & s, bitmap<Enum, N> & v, const Context & ctx = {})
    {
        adapters::BitMapAdapter<bitmap<Enum, N>> a(v);

        ReadVector(s, a, ctx);
    }

    template <class Stream, typename Enum, typename std::underlying_type<Enum>::type N, class Context = FakeContext>
    inline void Write(Stream & s, const bitmap<Enum, N> & v, const Context & ctx = {})
    {
        const adapters::BitMapAdapter<const bitmap<Enum, N>> a(v);

        WriteVector(s, a, ctx);
    }
}
