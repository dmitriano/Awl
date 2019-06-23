#pragma once

#include "Awl/Prototype.h"
#include "Awl/Io/RwHelpers.h"

#include <functional>
#include <assert.h>

namespace awl::io
{
    template <class StructV, class FieldV>
    class Context
    {
    public:

        template <class S>
        static auto MakeNewPrototype()
        {
            //This will trigger the static_assert if S is not in StructV.
            find_variant_type_v<S, StructV>;
            return AttachedPrototype<FieldV, S>();
        }

        template <class S>
        const Prototype & FindOldPrototype() const
        {
            constexpr size_t index = find_variant_type_v<S, StructV>;
            assert(index < oldPrototypes.size());
            return oldPrototypes[index];
        }

        template <class S>
        const std::vector<size_t> & FindProtoMap() const
        {
            constexpr size_t index = find_variant_type_v<S, StructV>;
            assert(index < oldPrototypes.size());
            return protoMaps[index];
        }

        //Makes the new and old prototypes identical.
        void Initialize()
        {
            assert(oldPrototypes.empty());
            auto a = MakeNewPrototypes();
            std::copy(a.begin(), a.end(), std::back_inserter(oldPrototypes));
            MakeProtoMaps();
        }
        
        template <class Stream>
        void ReadOld(Stream & s)
        {
            assert(oldPrototypes.empty());
            //Read as std::vector.
            Read(s, oldPrototypes);
            MakeProtoMaps();
        }

        template <class Stream>
        void WriteNew(Stream & s) const
        {
            //Write as std::array.
            auto a = MakeNewPrototypes();
            Write(s, a.size());
            Write(s, a);
        }

        template <class Stream>
        auto MakeFieldReaders() const
        {
            return MakeFieldReaders<Stream>(std::make_index_sequence<std::variant_size_v<FieldV>>());
        }

        const bool allowTypeMismatch = false;
        const bool allowDelete = true;

    private:

        typedef std::array<DetachedPrototype, std::variant_size_v<StructV>> NewArray;

        template <std::size_t... index>
        NewArray MakeNewPrototypes(std::index_sequence<index...>) const
        {
            return NewArray{ DetachedPrototype(MakeNewPrototype<std::variant_alternative_t<index, StructV>>())... };
        }

        NewArray MakeNewPrototypes() const
        {
            return MakeNewPrototypes(std::make_index_sequence<std::variant_size_v<StructV>>());
        }

        template <class Stream, size_t index>
        static constexpr auto MakeFieldReader()
        {
            return [](Stream & s)
            {
                std::variant_alternative_t<index, FieldV> val;
                Read(s, val);
                return FieldV(val);
            };
        }
        
        template <class Stream, std::size_t... index>
        auto MakeFieldReaders(std::index_sequence<index...>) const
        {
            typedef std::array<std::function<FieldV(Stream & s)>, std::variant_size_v<FieldV>> ReaderArray;
            return ReaderArray{ MakeFieldReader<Stream, index>() ... };
        }

        template <size_t index>
        void MakeProtoMap()
        {
            //can there be a better implementation?
            if (index < oldPrototypes.size())
            {
                auto new_proto = MakeNewPrototype<std::variant_alternative_t<index, StructV>>();
                auto & old_proto = oldPrototypes[index];
                protoMaps[index] = old_proto.MapNames(new_proto);
            }
        }
        
        void MakeProtoMaps()
        {
            assert(protoMaps.empty());
            constexpr size_t size = std::variant_size_v<StructV>;
            protoMaps.resize(size);
            return MakeProtoMaps(std::make_index_sequence<size>());
        }

        template <std::size_t... index>
        void MakeProtoMaps(std::index_sequence<index...>)
        {
            (MakeProtoMap<index>(), ...);
        }

        std::vector<DetachedPrototype> oldPrototypes;
        std::vector<std::vector<size_t>> protoMaps;
    };

    typedef Context<std::variant<>, std::variant<>> FakeContext;
}
