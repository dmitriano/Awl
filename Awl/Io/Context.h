#pragma once

#include "Awl/Prototype.h"
#include "Awl/Io/RwHelpers.h"
#include "Awl/Io/SequentialStream.h"

#include <assert.h>

namespace awl::io
{
    template <class Struct>
    struct FieldReader
    {
        virtual void ReadField(SequentialInputStream & in, Struct & val) const = 0;
    };

    template <class Struct, size_t index>
    class FieldReaderImpl : public FieldReader<Struct>
    {
    public:

        void ReadField(SequentialInputStream & in, Struct & val) const override
        {
            Read(in, std::get<index>(val.as_tuple()));
        }
    };

    struct FieldSkipper
    {
        virtual void SkipField(SequentialInputStream & in) const = 0;
    };

    template <class Field>
    class FieldSkipperImpl : public FieldSkipper
    {
    public:

        void SkipField(SequentialInputStream & in) const override
        {
            Field val;
            Read(in, val);
        }
    };

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

        template <class StructV>
        struct FieldReaderTupleCreator
        {
            template <class Struct, std::size_t... index>
            static auto MakeFieldReaderTuple(std::index_sequence<index...>)
            {
                return std::make_tuple(FieldReaderImpl<Struct, index>()...);
            }

            template <class Struct>
            static auto MakeFieldReaderTuple()
            {
                return MakeFieldReaderTuple<Struct>(std::make_index_sequence<std::tuple_size_v<tuplizable_traits<Struct>::Tie>>());
            }

            template <std::size_t... index>
            static auto MakeReaderTuple(std::index_sequence<index...>)
            {
                return std::make_tuple(MakeFieldReaderTuple<std::variant_alternative_t<index, StructV>>()...);
            }

            //Creates a tuple of tuples of FieldReader-s.
            static auto MakeReaderTuple()
            {
                return MakeReaderTuple(std::make_index_sequence<std::variant_size_v<StructV>>());
            }
        };
    }

    template <class StructV, class FieldV>
    class Context
    {
    private:

        typedef decltype(helpers::PrototypeTupleCreator<StructV, FieldV>::MakePrototypeTuple()) NewPrototypeTuple;
        typedef std::array<Prototype *, std::variant_size_v<StructV>> NewPrototypeArray;

        typedef decltype(helpers::FieldReaderTupleCreator<StructV>::MakeReaderTuple()) ReaderTuple;

        typedef decltype(transform_v2t<FieldV, FieldSkipperImpl>()) SkipperTuple;
        typedef std::array<FieldSkipper *, std::variant_size_v<FieldV>> SkipperArray;

        typedef std::array<std::function<FieldV(SequentialInputStream & s)>, std::variant_size_v<FieldV>> ReaderArray;
    
    public:

        Context() :
            newPrototypesTuple(helpers::PrototypeTupleCreator<StructV, FieldV>::MakePrototypeTuple()),
            newPrototypes(tuple_to_array(newPrototypesTuple, [](auto & field) { return static_cast<Prototype *>(&field); })),
            readerTuple(helpers::FieldReaderTupleCreator<StructV>::MakeReaderTuple()),
            skipperTuple(transform_v2t<FieldV, FieldSkipperImpl>()),
            skipperArray(tuple_to_array(skipperTuple, [](auto & field) { return static_cast<FieldSkipper *>(&field); }))
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
        auto MakeFieldReaders() const
        {
            constexpr size_t index = StructIndex<S>;
            auto & reader_tuple = std::get<index>(readerTuple);
            return tuple_to_array(reader_tuple, [](auto & reader) { return static_cast<const FieldReader<S> *>(&reader); });
        }

        const SkipperArray & GetFieldSkippers() const
        {
            return skipperArray;
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

        typedef uint16_t StructIndexType;
        
        bool serializeStructIndex = true;
        bool allowTypeMismatch = false;
        bool allowDelete = true;

    private:

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

        NewPrototypeTuple newPrototypesTuple;
        NewPrototypeArray newPrototypes;
        std::vector<DetachedPrototype> oldPrototypes;
        std::vector<std::vector<size_t>> protoMaps;
        ReaderTuple readerTuple;
        SkipperTuple skipperTuple;
        SkipperArray skipperArray;
    };

    typedef Context<std::variant<>, std::variant<>> FakeContext;
}
