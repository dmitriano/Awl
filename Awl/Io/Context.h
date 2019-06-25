#pragma once

#include "Awl/Prototype.h"
#include "Awl/Io/RwHelpers.h"
#include "Awl/Io/SequentialStream.h"

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
            newPrototypes(to_array(newPrototypesTuple, [](auto & field) { return static_cast<Prototype *>(&field); })),
            fieldReaders(MakeFieldReaders())
        {
        }

        template <class S>
        inline static constexpr size_t StructIndex = find_variant_type_v<S, StructV>;

        template <class S>
        const AttachedPrototype<FieldV, S> & FindNewPrototype() const
        {
            constexpr size_t index = StructIndex<S>;
            return std::get<index>(newPrototypesTuple);
        }

        template <class S>
        const bool HasOldPrototype() const
        {
            constexpr size_t index = StructIndex<S>;
            return index < oldPrototypes.size();
        }

        template <class S>
        const Prototype & FindOldPrototype() const
        {
            constexpr size_t index = StructIndex<S>;
            assert(index < oldPrototypes.size());
            return oldPrototypes[index];
        }

        template <class S>
        const std::vector<size_t> & FindProtoMap() const
        {
            constexpr size_t index = StructIndex<S>;
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
        
        void ReadOldPrototypes(SequentialInputStream & s)
        {
            assert(oldPrototypes.empty());
            //Read std::vector.
            Read(s, oldPrototypes);
            MakeProtoMaps();
        }

        void WriteNewPrototypes(SequentialOutputStream & s) const
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

        auto & GetFieldReader(size_t old_index) const
        {
            assert(old_index < fieldReaders.size());
            return fieldReaders[old_index];
        }

        typedef uint16_t StructIndexType;
        
        bool serializeStructIndex = true;
        bool allowTypeMismatch = false;
        bool allowDelete = true;

    private:

        typedef decltype(helpers::PrototypeTupleCreator<StructV, FieldV>::MakePrototypeTuple()) NewTuple;
        typedef std::array<Prototype *, std::variant_size_v<StructV>> NewArray;
        typedef std::array<std::function<FieldV(SequentialInputStream & s)>, std::variant_size_v<FieldV>> ReaderArray;

        auto MakeFieldReaders() const
        {
            return MakeFieldReaders(std::make_index_sequence<std::variant_size_v<FieldV>>());
        }

        template <std::size_t... index>
        auto MakeFieldReaders(std::index_sequence<index...>) const
        {
            return ReaderArray{
                [](SequentialInputStream & s)
                {
                    std::variant_alternative_t<index, FieldV> val;
                    Read(s, val);
                    return FieldV(val);
                }
                ...
            };
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
        ReaderArray fieldReaders;
    };

    typedef Context<std::variant<>, std::variant<>> FakeContext;
}
