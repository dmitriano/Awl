#pragma once

#include "Awl/Io/Rw/ReadRaw.h"
#include "Awl/TupleHelpers.h"

#include <variant>
#include <type_traits>
#include <cassert>

namespace awl::io
{
    namespace impl
    {
        template <class V, class Stream, class Context>
        struct VariantReader
        {
            virtual void ReadElement(Stream & in, V & v, const Context & ctx) const = 0;
        };

        template <class V, class Stream, class Context, size_t index>
        class VariantReaderImpl : public VariantReader<V, Stream, Context>
        {
        public:

            void ReadElement(Stream & in, V & v, const Context & ctx) const override
            {
                using T = std::variant_alternative_t<index, V>;
                
                T val;
                Read(in, val, ctx);
                v = val;
            }
        };
    }
    
    template <class Stream, typename... Ts, class Context = FakeContext>
    inline void Read(Stream & s, std::variant<Ts...> & v, const Context & ctx = {})
    {
        using V = std::variant<Ts...>;

        static auto vrt = transform_v2t<V, impl::VariantReaderImpl<V, Stream, Context>>();
        static auto vra = tuple_cast<impl::VariantReader<V, Stream, Context>>(vrt);

        std::size_t index;
        Read(s, index, ctx);

        assert(index < std::variant_size_v<V>);

        vra[index]->ReadElement(in, v, ctx);
    }

    template <class Stream, typename... Ts, class Context = FakeContext>
    inline void Write(Stream & s, std::variant<Ts...> & v, const Context & ctx = {})
    {
    }
}
