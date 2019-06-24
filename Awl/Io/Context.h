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
            
            auto new_protos = MakeNewPrototypes();

            for (Prototype * p : new_protos.a)
            {
                oldPrototypes.push_back(DetachedPrototype(*p));
            }
            
            MakeProtoMaps();
        }
        
        template <class Stream>
        void ReadOld(Stream & s)
        {
            assert(oldPrototypes.empty());
            //Read std::vector.
            Read(s, oldPrototypes);
            MakeProtoMaps();
        }

        template <class Stream>
        void WriteNew(Stream & s) const
        {
            //Write std::array.
            auto new_protos = MakeNewPrototypes();
            Write(s, new_protos.a.size());
            
            for (Prototype * p : new_protos.a)
            {
                for (size_t i = 0; i < p->GetCount(); ++i)
                {
                    FieldRef fr = p->GetField(i);
                    Write(s, fr.name);
                    Write(s, fr.type);
                }
            }
        }

        template <class Stream>
        auto MakeFieldReaders() const
        {
            return MakeFieldReaders<Stream>(std::make_index_sequence<std::variant_size_v<FieldV>>());
        }

        const bool allowTypeMismatch = false;
        const bool allowDelete = true;

    private:

        typedef std::array<Prototype *, std::variant_size_v<StructV>> NewArray;

        template <class Tuple, class Array>
        struct InterfaceArray
        {
            InterfaceArray(Tuple && tt, Array && aa) : t(std::move(tt)), a(std::move(aa))
            {
            }

            Tuple t;
            Array a;
        };

        template <std::size_t... index>
        auto MakeNewPrototypes(std::index_sequence<index...>) const
        {
            auto t = std::make_tuple(MakeNewPrototype<std::variant_alternative_t<index, StructV>>()... );
            auto a = to_array(t, [](auto & field) { return static_cast<Prototype *>(&field); });

            return InterfaceArray(std::move(t), std::move(a));
        }

        auto MakeNewPrototypes() const
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

        void MakeProtoMaps()
        {
            assert(protoMaps.empty());
            constexpr size_t size = std::variant_size_v<StructV>;
            protoMaps.resize(size);

            auto new_protos = MakeNewPrototypes();

            assert(new_protos.a.size() < oldPrototypes.size());

            for (size_t i = 0; i < oldPrototypes.size(); ++i)
            {
                protoMaps[i] = oldPrototypes[i].MapNames(*(new_protos.a[i]));
            }
        }

        std::vector<DetachedPrototype> oldPrototypes;
        std::vector<std::vector<size_t>> protoMaps;
    };

    typedef Context<std::variant<>, std::variant<>> FakeContext;
}
