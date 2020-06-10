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
        struct VariantAlternativeReader
        {
            virtual void ReadAlternative(Stream & in, V & v, const Context & ctx) const = 0;
        };

        template <class V, class Stream, class Context, size_t index>
        class VariantAlternativeReaderImpl : public VariantAlternativeReader<V, Stream, Context>
        {
        public:

            void ReadAlternative(Stream & in, V & v, const Context & ctx) const override
            {
                using T = std::variant_alternative_t<index, V>;
                
                T val;
                Read(in, val, ctx);
                v = val;
            }
        };

        template <class V, class Stream, class Context>
        class VariantReader
        {
        private:

            template <size_t index>
            using AlternativeReaderImpl = VariantAlternativeReaderImpl<V, Stream, Context, index>;

        public:

            static void ReadVariant(Stream & in, V & v, const Context & ctx)
            {
                static auto vrt = transform_v2ti<V, AlternativeReaderImpl>();
                static auto vra = tuple_cast<VariantAlternativeReader<V, Stream, Context>>(vrt);

                std::size_t index;
                Read(in, index, ctx);

                assert(index < std::variant_size_v<V>);

                vra[index]->ReadAlternative(in, v, ctx);
            }
        };

        template <class V, class Stream, class Context>
        struct VariantAlternativeWriter
        {
            virtual void WriteAlternative(Stream & out, const V & v, const Context & ctx) const = 0;
        };

        template <class V, class Stream, class Context, size_t index>
        class VariantAlternativeWriterImpl : public VariantAlternativeWriter<V, Stream, Context>
        {
        public:

            void WriteAlternative(Stream & out, const V & v, const Context & ctx) const override
            {
                using T = std::variant_alternative_t<index, V>;

                const T & val = std::get<index>(v);
                Write(out, val, ctx);
            }
        };

        template <class V, class Stream, class Context>
        class VariantWriter
        {
        private:

            template <size_t index>
            using AlternativeWriterImpl = VariantAlternativeWriterImpl<V, Stream, Context, index>;

        public:

            static void WriteVariant(Stream & out, const V & v, const Context & ctx)
            {
                static auto vrt = transform_v2ti<V, AlternativeWriterImpl>();
                static auto vra = tuple_cast<VariantAlternativeWriter<V, Stream, Context>>(vrt);

                const std::size_t index = v.index();
                Write(out, index, ctx);

                vra[index]->WriteAlternative(out, v, ctx);
            }
        };
    }
    
    template <class Stream, typename... Ts, class Context = FakeContext>
    inline void Read(Stream & s, std::variant<Ts...> & v, const Context & ctx = {})
    {
        using V = std::variant<Ts...>;

        impl::VariantReader<V, Stream, Context>::ReadVariant(s, v, ctx);
    }

    template <class Stream, typename... Ts, class Context = FakeContext>
    inline void Write(Stream & s, std::variant<Ts...> & v, const Context & ctx = {})
    {
        using V = std::variant<Ts...>;

        impl::VariantWriter<V, Stream, Context>::WriteVariant(s, v, ctx);
    }
}
