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

    template <class StructV, class FieldV>
    class Context
    {
    private:

        template <class Struct>
        using MyAttachedPrototype = AttachedPrototype<FieldV, Struct>;

        typedef decltype(transform_v2t<StructV, MyAttachedPrototype>()) NewPrototypeTuple;
        typedef std::array<Prototype *, std::variant_size_v<StructV>> NewPrototypeArray;

        template <class Struct>
        struct FieldReaderTupleCreator
        {
            static constexpr size_t fieldCount = std::tuple_size_v<tuplizable_traits<Struct>::Tie>;
            
            template <std::size_t... index>
            static auto MakeTuple(std::index_sequence<index...>)
            {
                return std::make_tuple(FieldReaderImpl<Struct, index>()...);
            }

            static auto MakeTuple()
            {
                return MakeTuple(std::make_index_sequence<fieldCount>());
            }
        };

        template <class Struct>
        struct FieldReadersHolder
        {
            typedef decltype(FieldReaderTupleCreator<Struct>::MakeTuple()) Tuple;
            typedef std::array<const FieldReader<Struct> *, FieldReaderTupleCreator<Struct>::fieldCount> Array;

            FieldReadersHolder() :
                t(FieldReaderTupleCreator<Struct>::MakeTuple())
            {
                //The object will be copied or moved after construction, it does not make a sense to initialize the array in the constructor.
                std::fill(a.begin(), a.end(), nullptr);
            }

            void InitializeArray() const
            {
                a = tuple_to_array(t, [](auto & reader) { return dynamic_cast<const FieldReader<Struct> *>(&reader); });
            }

            Tuple t;
            mutable Array a;
        };

        typedef decltype(transform_v2t<StructV, FieldReadersHolder>()) ReaderTuple;

        typedef decltype(transform_v2t<FieldV, FieldSkipperImpl>()) SkipperTuple;
        typedef std::array<FieldSkipper *, std::variant_size_v<FieldV>> SkipperArray;

    
    public:

        Context() :
            newPrototypesTuple(transform_v2t<StructV, MyAttachedPrototype>()),
            newPrototypes(tuple_to_array(newPrototypesTuple, [](auto & field) { return static_cast<Prototype *>(&field); })),
            readerTuple(transform_v2t<StructV, FieldReadersHolder>()),
            skipperTuple(transform_v2t<FieldV, FieldSkipperImpl>()),
            skipperArray(tuple_to_array(skipperTuple, [](auto & field) { return static_cast<FieldSkipper *>(&field); }))
        {
            for_each(readerTuple, [](auto & holder) { holder.InitializeArray(); });
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
        auto & FindFieldReaders() const
        {
            constexpr size_t index = StructIndex<S>;
            auto & holder = std::get<index>(readerTuple);
            return holder.a;
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
