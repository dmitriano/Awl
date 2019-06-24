#pragma once

#include "Awl/Prototype.h"
#include "Awl/Io/RwHelpers.h"

#include <functional>
#include <assert.h>

namespace awl::io
{
    namespace helpers
    {
        template <class StructV, class FieldV>
        struct PrototypeTupleCreator
        {
            template <class S>
            static auto MakePrototype()
            {
                //This will trigger the static_assert if S is not in StructV.
                find_variant_type_v<S, StructV>;
                return AttachedPrototype<FieldV, S>();
            }

            template <std::size_t... index>
            static auto MakePrototypeTuple(std::index_sequence<index...>)
            {
                return std::make_tuple(MakePrototype<std::variant_alternative_t<index, StructV>>()...);
            }

            static auto MakePrototypeTuple()
            {
                return MakePrototypeTuple(std::make_index_sequence<std::variant_size_v<StructV>>());
            }
        };
    }

    template <class StructV, class FieldV>
    class Context
    {
    public:

        Context() :
            newPrototypesTuple(helpers::PrototypeTupleCreator<StructV, FieldV>::MakePrototypeTuple()),
            newPrototypes(to_array(newPrototypesTuple, [](auto & field) { return static_cast<Prototype *>(&field); }))
        {
        }

        template <class S>
        static auto MakeNewPrototype()
        {
            return helpers::PrototypeTupleCreator<StructV, FieldV>::MakePrototype<S>();
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
            
            for (Prototype * p : newPrototypes)
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
            Write(s, newPrototypes.size());
            
            for (Prototype * p : newPrototypes)
            {
                const size_t count = p->GetCount();
                Write(s, count);
                
                for (size_t i = 0; i < count; ++i)
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

        typedef decltype(helpers::PrototypeTupleCreator<StructV, FieldV>::MakePrototypeTuple()) NewTuple;
        typedef std::array<Prototype *, std::variant_size_v<StructV>> NewArray;

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

            assert(oldPrototypes.size() <= newPrototypes.size());

            for (size_t i = 0; i < oldPrototypes.size(); ++i)
            {
                protoMaps[i] = oldPrototypes[i].MapNames(*(newPrototypes[i]));
            }
        }

        NewTuple newPrototypesTuple;
        NewArray newPrototypes;
        std::vector<DetachedPrototype> oldPrototypes;
        std::vector<std::vector<size_t>> protoMaps;
    };

    typedef Context<std::variant<>, std::variant<>> FakeContext;
}
