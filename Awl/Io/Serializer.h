#pragma once

#include "Awl/Prototype.h"
#include "Awl/Io/RwHelpers.h"
#include "Awl/Io/SequentialStream.h"
#include "Awl/Stringizable.h"
#include "Awl/TupleHelpers.h"
#include "Awl/IntRange.h"
#include "Awl/Io/IoException.h"
#include "Awl/Io/RwHelpers.h"
#include "Awl/Io/MpHelpers.h"

#include <cassert>

namespace awl::io
{
    template <class V>
    class Serializer
    {
    protected:

        using StructV = helpers::filter_variant<V, is_stringizable>;
        using FieldV = V;

        using StructIndexType = uint16_t;

        template <class S>
        static constexpr size_t StructIndex = find_variant_type_v<S, StructV>;

        template <class Struct>
        using MyAttachedPrototype = AttachedPrototype<FieldV, Struct>;

        using NewPrototypeTuple = decltype(transform_v2t<StructV, MyAttachedPrototype>());
        using NewPrototypeArray = std::array<Prototype *, std::variant_size_v<StructV>>;

        NewPrototypeTuple newPrototypesTuple;
        NewPrototypeArray newPrototypes;

        Serializer() :
            newPrototypesTuple(transform_v2t<StructV, MyAttachedPrototype>()),
            newPrototypes(tuple_cast<Prototype>(newPrototypesTuple))
        {
        }

    public:

        //It contains the addresses of its members.
        Serializer(const Serializer&) = delete;
        Serializer(Serializer&&) = delete;
        Serializer& operator = (const Serializer&) = delete;
        Serializer& operator = (Serializer&&) = delete;

        template <class S>
        const AttachedPrototype<FieldV, S> & FindNewPrototype() const
        {
            constexpr size_t index = StructIndex<S>;
            return std::get<index>(newPrototypesTuple);
        }

        bool allowTypeMismatch = false;
        bool allowDelete = true;
    };

    template <class V, class IStream = SequentialInputStream>
    class Reader : public Serializer<V>
    {
    public:

        using InputStream = IStream;

    private:

        using Base = Serializer<V>;

        template <class Struct>
        struct FieldReader
        {
            virtual void ReadField(const Reader & context, InputStream & in, Struct & val) const = 0;
        };

        template <class Struct, size_t index>
        class FieldReaderImpl : public FieldReader<Struct>
        {
        public:

            void ReadField(const Reader & context, InputStream & in, Struct & val) const override
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
            virtual void SkipField(const Reader & context, InputStream & in) const = 0;
        };

        template <class Field>
        class FieldSkipperImpl : public FieldSkipper
        {
        public:

            void SkipField(const Reader & context, InputStream & in) const override
            {
                Field val;
                context.ReadV(in, val);
            }
        };

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
            using Struct = std::variant_alternative_t<index, typename Base::StructV>;
            
            FieldReaderArrayHolder(const FieldReaderTuple<Struct> & t) :
                a(tuple_cast<const FieldReader<Struct>>(t))
            {
            }
            
            FieldReaderArray<Struct> a;
        };
        
        using TupleOfFieldReaderTuple = decltype(transform_v2t<typename Base::StructV, FieldReaderTuple>());
        using TupleOfFieldReaderArray = decltype(transform_t2ti<FieldReaderArrayHolder>(TupleOfFieldReaderTuple{}));

        using SkipperTuple = decltype(transform_v2t<typename Base::FieldV, FieldSkipperImpl>());
        using SkipperArray = std::array<FieldSkipper *, std::variant_size_v<typename Base::FieldV>>;
    
    public:

        Reader() :
            readerTuples(transform_v2t<typename Base::StructV, FieldReaderTuple>()),
            readerArrays(transform_t2ti<FieldReaderArrayHolder>(readerTuples)),
            skipperTuple(transform_v2t<typename Base::FieldV, FieldSkipperImpl>()),
            skipperArray(tuple_cast<FieldSkipper>(skipperTuple))
        {
        }

        //Makes the new and old prototypes identical.
        void Initialize()
        {
            assert(oldPrototypes.empty());
            
            for (Prototype * p : this->newPrototypes)
            {
                oldPrototypes.push_back(DetachedPrototype(*p));
            }
            
            MakeProtoMaps();
        }
        
        template <class Stream>
        void ReadOldPrototypes(Stream & s)
        {
            assert(oldPrototypes.empty());
            //Read std::vector.
            Read(s, oldPrototypes);
            MakeProtoMaps();
        }

        template<class Struct>
        void ReadV(InputStream & s, Struct & val) const
        {
            if constexpr (is_stringizable_v<Struct>)
            {
                typename Base::StructIndexType old_struct_index = ReadStructIndex(s);

                const std::vector<size_t> & name_map = protoMaps[old_struct_index];

                //An empty map means either an empty structure or equal prototypes
                //(the prototypes of empty structures are equal).
                if (name_map.empty())
                {
                    //Read in the same way we write it.
                    ReadTuplizable(s, val);
                }
                else
                {
                    auto & new_proto = this->template FindNewPrototype<Struct>();
                    auto & old_proto = oldPrototypes[old_struct_index];

                    assert(name_map.size() == old_proto.GetCount());

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
                                throw FieldNotFoundException(std::string(old_field.name));
                            }

                            //Skip by type.
                            skippers[old_field.type]->SkipField(*this, s);
                        }
                        else
                        {
                            const auto new_field = new_proto.GetField(new_index);

                            if (new_field.type != old_field.type)
                            {
                                throw TypeMismatchException(std::string(new_field.name), new_field.type, old_field.type);
                            }

                            //But read by index.
                            readers[new_index]->ReadField(*this, s, val);
                        }
                    }
                }
            }
            else if constexpr (is_tuplizable_v<Struct>)
            {
                ReadTuplizable(s, val);
            }
            else
            {
                Read(s, val);
            }
        }

        //Reads entire object tree assuming all the prototypes are equal.
        template<class Struct>
        void ReadPlain(InputStream & s, Struct & val) const
        {
            if constexpr (is_stringizable_v<Struct>)
            {
                ReadStructIndex(s);
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

    private:

        template <class S>
        auto & FindFieldReaders() const
        {
            constexpr size_t index = Base::template StructIndex<S>;
            auto & holder = std::get<index>(readerArrays);
            return holder.a;
        }

        const SkipperArray & GetFieldSkippers() const
        {
            return skipperArray;
        }

        typename Base::StructIndexType ReadStructIndex(InputStream & s) const
        {
            typename Base::StructIndexType index;
            Read(s, index);
            assert(index < oldPrototypes.size());
            return index;
        }

        template<class Struct>
        void ReadTuplizable(InputStream & s, Struct & val) const
        {
            for_each(object_as_tuple(val), [this, &s](auto& field_val)
            {
                //A tuplizable structure field can be serializable.
                ReadV(s, field_val);
            });
        }

        void MakeProtoMaps()
        {
            assert(protoMaps.empty());
            constexpr size_t size = std::variant_size_v<typename Base::StructV>;
            protoMaps.resize(size);

            assert(oldPrototypes.size() <= this->newPrototypes.size());

            for (size_t i = 0; i < oldPrototypes.size(); ++i)
            {
                std::vector<size_t> v = oldPrototypes[i].MapNames(*(this->newPrototypes[i]));

                //Clear the vector if the map is trivial.
                auto range = awl::make_count(v.size());
                if (std::equal(v.begin(), v.end(), range.begin(), range.end()))
                {
                    v.clear();
                }

                protoMaps[i] = v;
            }
        }

        std::vector<DetachedPrototype> oldPrototypes;
        std::vector<std::vector<size_t>> protoMaps;
        TupleOfFieldReaderTuple readerTuples;
        TupleOfFieldReaderArray readerArrays;
        SkipperTuple skipperTuple;
        SkipperArray skipperArray;
    };

    template <class V, class OStream = SequentialOutputStream>
    class Writer : public Serializer<V>
    {
    private:

        using Base = Serializer<V>;

    public:

        using OutputStream = OStream;

        template <class Stream>
        void WriteNewPrototypes(Stream & s) const
        {
            //Write std::array.
            Write(s, this->newPrototypes.size());

            for (Prototype * p : this->newPrototypes)
            {
                const size_t count = p->GetCount();
                Write(s, count);

                for (size_t i = 0; i < count; ++i)
                {
                    Field f = p->GetField(i);
                    //Write name as string_view but read as string.
                    const size_t len = f.name.length();
                    Write(s, len);
                    s.Write(reinterpret_cast<const uint8_t *>(f.name.data()), len * sizeof(char));
                    Write(s, f.type);
                }
            }
        }

        //Writes the object tree and adds indices to the structures.
        template<class Struct>
        void WriteV(OutputStream & s, const Struct & val) const
        {
            if constexpr (is_stringizable_v<Struct>)
            {
                const typename Base::StructIndexType index = static_cast<typename Base::StructIndexType>(Base::template StructIndex<Struct>);
                Write(s, index);
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
    };
}
