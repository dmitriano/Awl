#pragma once

#include "Awl/Prototype.h"
#include "Awl/Io/RwHelpers.h"
#include "Awl/Io/SequentialStream.h"
#include "Awl/Stringizable.h"
#include "Awl/TupleHelpers.h"
#include "Awl/Io/IoException.h"
#include "Awl/Io/RwHelpers.h"
#include "Awl/IntRange.h"

#include <cassert>

namespace awl::io
{
    template <class V, class IStream = SequentialInputStream, class OStream = SequentialOutputStream>
    class Serializer
    {
    public:

        using InputStream = IStream;
        using OutputStream = OStream;

    private:

        using Split = split_variant<V, is_stringizable>;
        using StructV = typename Split::matching;
        using FieldV = V;

        template <class Struct>
        struct FieldReader
        {
            virtual void ReadField(const Serializer & context, InputStream & in, Struct & val) const = 0;
        };

        template <class Struct, size_t index>
        class FieldReaderImpl : public FieldReader<Struct>
        {
        public:

            void ReadField(const Serializer & context, InputStream & in, Struct & val) const override
            {
                auto & field_val = std::get<index>(val.as_tuple());

                if constexpr (is_stringizable_v<std::remove_reference_t<decltype(field_val)>>)
                {
                    context.ReadV(in, field_val);
                }
                else
                {
                    static_cast<void>(context);
                    Read(in, field_val);
                }
            }
        };

        struct FieldSkipper
        {
            virtual void SkipField(InputStream & in) const = 0;
        };

        template <class Field>
        class FieldSkipperImpl : public FieldSkipper
        {
        public:

            void SkipField(InputStream & in) const override
            {
                Field val;
                Read(in, val);
            }
        };

        template <class Struct>
        using MyAttachedPrototype = AttachedPrototype<FieldV, Struct>;

        using NewPrototypeTuple = decltype(transform_v2t<StructV, MyAttachedPrototype>());
        using NewPrototypeArray = std::array<Prototype *, std::variant_size_v<StructV>>;

        template <class Struct>
        struct FieldReaderTupleCreator
        {
            static constexpr size_t fieldCount = std::tuple_size_v<typename tuplizable_traits<Struct>::Tie>;
            
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
        using FieldReaderTuple = decltype(FieldReaderTupleCreator<Struct>::MakeTuple());
        
        template <class Struct>
        using FieldReaderArray = std::array<const FieldReader<Struct> *, FieldReaderTupleCreator<Struct>::fieldCount>;

        template <size_t index>
        struct FieldReaderArrayHolder
        {
            using Struct = std::variant_alternative_t<index, StructV>;
            
            FieldReaderArrayHolder(const FieldReaderTuple<Struct> & t) :
                a(tuple_cast<const FieldReader<Struct>>(t))
            {
            }
            
            FieldReaderArray<Struct> a;
        };
        
        using TupleOfFieldReaderTuple = decltype(transform_v2t<StructV, FieldReaderTuple>());
        using TupleOfFieldReaderArray = decltype(transform_t2ti<FieldReaderArrayHolder>(TupleOfFieldReaderTuple{}));

        using SkipperTuple = decltype(transform_v2t<FieldV, FieldSkipperImpl>());
        using SkipperArray = std::array<FieldSkipper *, std::variant_size_v<FieldV>>;
    
    public:

        Serializer() :
            newPrototypesTuple(transform_v2t<StructV, MyAttachedPrototype>()),
            newPrototypes(tuple_cast<Prototype>(newPrototypesTuple)),
            readerTuples(transform_v2t<StructV, FieldReaderTuple>()),
            readerArrays(transform_t2ti<FieldReaderArrayHolder>(readerTuples)),
            skipperTuple(transform_v2t<FieldV, FieldSkipperImpl>()),
            skipperArray(tuple_cast<FieldSkipper>(skipperTuple))
        {
        }

        //It contains the addresses of its members.
        Serializer(const Serializer&) = delete;
        Serializer(Serializer&&) = delete;
        Serializer& operator = (const Serializer&) = delete;
        Serializer& operator = (Serializer&&) = delete;

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
            auto & holder = std::get<index>(readerArrays);
            return holder.a;
        }

        const SkipperArray & GetFieldSkippers() const
        {
            return skipperArray;
        }

        template <class S>
        bool HasOldPrototype() const
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
        
        void ReadOldPrototypes(InputStream & s)
        {
            assert(oldPrototypes.empty());
            //Read std::vector.
            Read(s, oldPrototypes);
            MakeProtoMaps();
        }

        void WriteNewPrototypes(OutputStream & s) const
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

        using StructIndexType = uint16_t;
        
        bool serializeStructIndex = true;
        bool allowTypeMismatch = false;
        bool allowDelete = true;

        template<class Struct>
        void ReadStructIndex(InputStream & s, Struct & val) const
        {
            static_cast<void>(val);
            
            if (this->serializeStructIndex)
            {
                StructIndexType index;
                Read(s, index);
                constexpr size_t expected_index = StructIndex<Struct>;
                if (index != expected_index)
                {
                    throw TypeMismatchException(typeid(Struct).name(), index, expected_index);
                }
            }
        }

        template<class Struct>
        void ReadV(InputStream & s, Struct & val) const
        {
            ReadStructIndex(s, val);

            const std::vector<size_t> & name_map = this->template FindProtoMap<Struct>();

            //An empty map means either an empty structure or equal prototypes
            //(the prototypes of empty structures are equal).
            if (name_map.empty())
            {
                //Read in the same way we write it.
                for_each(object_as_tuple(val), [this, &s](auto& field_val)
                {
                    if constexpr (is_stringizable_v<std::remove_reference_t<decltype(field_val)>>)
                    {
                        ReadV(s, field_val);
                    }
                    else
                    {
                        Read(s, field_val);
                    }
                });
            }
            else
            {
                assert(name_map.size() == old_proto.GetCount());

                auto & new_proto = this->template FindNewPrototype<Struct>();
                auto & old_proto = this->template FindOldPrototype<Struct>();

                auto & readers = this->template FindFieldReaders<Struct>();
                auto & skippers = this->GetFieldSkippers();

                for (size_t old_index = 0; old_index < name_map.size(); ++old_index)
                {
                    const auto old_field = old_proto.GetField(old_index);

                    const size_t new_index = name_map[old_index];

                    if (new_index == Prototype::NoIndex)
                    {
                        if (!this->allowDelete)
                        {
                            throw FieldNotFoundException(old_field.name);
                        }

                        //Skip by type.
                        skippers[old_field.type]->SkipField(s);
                    }
                    else
                    {
                        const auto new_field = new_proto.GetField(new_index);

                        if (new_field.type != old_field.type)
                        {
                            throw TypeMismatchException(new_field.name, new_field.type, old_field.type);
                        }

                        //But read by index.
                        readers[new_index]->ReadField(*this, s, val);
                    }
                }
            }
        }

        //Reads entire object tree assuming all the prototypes are equal.
        template<class Struct>
        void ReadPlain(InputStream & s, Struct & val) const
        {
            if constexpr (is_stringizable_v<Struct>)
            {
                ReadStructIndex(s, val);
            }

            if constexpr (is_tuplizable_v<Struct>)
            {
                for_each(object_as_tuple(val), [this, &s](auto& field)
                {
                    ReadPlain(s, field);
                });
            }
            else
            {
                Read(s, val);
            }
        }

        //Writes the object tree and adds indices to the structures.
        template<class Struct>
        void WriteV(OutputStream & s, const Struct & val) const
        {
            if constexpr (is_stringizable_v<Struct>)
            {
                if (this->serializeStructIndex)
                {
                    const StructIndexType index = static_cast<StructIndexType>(StructIndex<Struct>);
                    Write(s, index);
                }
            }

            if constexpr (is_tuplizable_v<Struct>)
            {
                for_each(object_as_tuple(val), [this, &s](auto& field)
                {
                    WriteV(s, field);
                });
            }
            else
            {
                Write(s, val);
            }
        }

    private:

        void MakeProtoMaps()
        {
            assert(protoMaps.empty());
            constexpr size_t size = std::variant_size_v<StructV>;
            protoMaps.resize(size);

            assert(oldPrototypes.size() <= newPrototypes.size());

            for (size_t i = 0; i < oldPrototypes.size(); ++i)
            {
                std::vector<size_t> v = oldPrototypes[i].MapNames(*(newPrototypes[i]));

                //Clear the vector if the map is trivial.
                auto range = awl::make_count(v.size());
                if (std::equal(v.begin(), v.end(), range.begin(), range.end()))
                {
                    v.clear();
                }

                protoMaps[i] = v;
            }
        }

        NewPrototypeTuple newPrototypesTuple;
        NewPrototypeArray newPrototypes;
        std::vector<DetachedPrototype> oldPrototypes;
        std::vector<std::vector<size_t>> protoMaps;
        TupleOfFieldReaderTuple readerTuples;
        TupleOfFieldReaderArray readerArrays;
        SkipperTuple skipperTuple;
        SkipperArray skipperArray;
    };
}
