#pragma once

#include "Awl/Prototype.h"
#include "Awl/Io/ReadWrite.h"
#include "Awl/Io/SequentialStream.h"
#include "Awl/Stringizable.h"
#include "Awl/TupleHelpers.h"
#include "Awl/IntRange.h"
#include "Awl/Io/IoException.h"
#include "Awl/Io/MpHelpers.h"
#include "Awl/Io/TypeName.h"
#include "Awl/Io/Metadata.h"

#include <cassert>
#include <unordered_map>
#include <optional>

namespace awl::io
{
    template <class V>
    class Serializer
    {
    public:

        using Variant = V;

    protected:

        using Split = helpers::split_variant<V, is_stringizable>;

        using StructV = typename Split::matching;
        using FieldV = typename Split::non_matching;

        using StructIndexType = uint16_t;

        template <class S>
        static constexpr size_t StructIndex = find_variant_type_v<S, StructV>;

        template <class Struct>
        using MyAttachedPrototype = AttachedPrototype<FieldV, Struct>;

        using NewPrototypeTuple = decltype(transform_v2t<StructV, MyAttachedPrototype>());
        using NewPrototypeArray = std::array<Prototype *, std::variant_size_v<StructV>>;

        NewPrototypeTuple newPrototypesTuple;
        NewPrototypeArray newPrototypes;

        using I2nMap = TypeNameVector;
        using N2iMap = std::unordered_map<std::string, size_t>;

        class TypeMapBuilder
        {
        private:

            static constexpr size_t typeCount = std::variant_size_v<FieldV>;

            static constexpr auto MakeSequence()
            {
                return std::make_index_sequence<typeCount>();
            }

            template <std::size_t index>
            static std::string MakeName()
            {
                return make_type_name<std::variant_alternative_t<index, FieldV>>();
            }

            template <std::size_t... index>
            static I2nMap BuildI2nMap(std::index_sequence<index...>)
            {
                I2nMap tm;
                (tm.push_back(MakeName<index>()), ...);
                return tm;
            }

            template <std::size_t... index>
            static N2iMap BuildN2iMap(std::index_sequence<index...>)
            {
                N2iMap tm;
                (tm.emplace(MakeName<index>(), index), ...);
                return tm;
            }

        public:

            static I2nMap BuildI2nMap()
            {
                return BuildI2nMap(MakeSequence());
            }

            static N2iMap BuildN2iMap()
            {
                return BuildN2iMap(MakeSequence());
            }
        };

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
    };

    template <class V, class IStream = SequentialInputStream>
    class BasicReader : public Serializer<V>
    {
    private:

        using Base = Serializer<V>;

    public:

        using InputStream = IStream;

    protected:

        typename Base::StructIndexType ReadStructIndex(InputStream & s) const
        {
            typename Base::StructIndexType index;
            Read(s, index);
            return index;
        }
    };

    template <class V, class IStream = SequentialInputStream>
    class PlainReader : public BasicReader<V, IStream>
    {
    private:

        using Base = BasicReader<V, IStream>;

    public:

        using InputStream = IStream;

        //Reads entire object tree assuming all the prototypes are equal.
        template<class Struct>
        void ReadV(InputStream & s, Struct & val) const
        {
            if constexpr (is_stringizable_v<Struct>)
            {
                this->ReadStructIndex(s);
            }

            if constexpr (is_tuplizable_v<Struct>)
            {
                for_each(object_as_tuple(val), [this, &s](auto& field)
                {
                    ReadV(s, field);
                });
            }
            else
            {
                Read(s, val, *this);
            }
        }
    };

    template <class V, class IStream = SequentialInputStream>
    class Reader : public BasicReader<V, IStream>
    {
    public:

        using InputStream = IStream;

    private:

        using Base = BasicReader<V, IStream>;

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
                    Read(in, field_val, context);
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

        struct ProtoMap
        {
            size_t newStructIndex;
            std::vector<size_t> fieldMap;
        };
        
    public:

        Reader() :
            readerTuples(transform_v2t<typename Base::StructV, FieldReaderTuple>()),
            readerArrays(transform_t2ti<FieldReaderArrayHolder>(readerTuples)),
            skipperTuple(transform_v2t<typename Base::FieldV, FieldSkipperImpl>()),
            skipperArray(tuple_cast<FieldSkipper>(skipperTuple)),
            typeMap(Base::TypeMapBuilder::BuildI2nMap())
        {
        }

        //Makes the new and old prototypes identical.
        void Initialize()
        {
            //Type map is trivial, so we do not use it.

            assert(oldPrototypes.empty());

            for (Prototype * p : this->newPrototypes)
            {
                oldPrototypes.push_back(DetachedPrototype(*p));
            }
        }

        template <class Stream>
        void ReadOldPrototypes(Stream & s)
        {
            Metadata meta;
            
            Read(s, meta);
            
            AttachMetadata(meta);
        }

        void AttachMetadata(Metadata & meta)
        {
            const typename Base::I2nMap old_tm = meta.typeNames;

            assert(oldPrototypes.empty());
            std::vector<DetachedPrototype> protos = meta.prototypes;

            typename Base::N2iMap new_tm = Base::TypeMapBuilder::BuildN2iMap();

            for (DetachedPrototype & old_proto : protos)
            {
                for (size_t old_index = 0; old_index < old_proto.GetCount(); ++old_index)
                {
                    const auto old_field = old_proto.GetField(old_index);

                    if (old_field.type != Field::NoType)
                    {
                        if (old_field.type >= old_tm.size())
                        {
                            //The type table is corrupted.
                            throw CorruptionException();
                        }

                        const std::string & name = old_tm[old_field.type];

                        auto new_i = new_tm.find(name);

                        if (new_i == new_tm.end())
                        {
                            //Old type not found in new type table.
                            throw TypeMismatchException(std::string(old_field.name), old_field.type, Field::NoType);
                        }

                        size_t new_type = new_i->second;

                        if (new_type == Field::NoType)
                        {
                            //A scalar field cannot be mapped to a structure.
                            throw TypeMismatchException(std::string(old_field.name), old_field.type, Field::NoType);
                        }

                        old_proto.SetFieldType(old_index, new_type);
                    }
                }
            }

            oldPrototypes = protos;
        }

        void ClearPrototypes()
        {
            oldPrototypes.clear();
            protoMaps.clear();
        }

        template<class Struct>
        void ReadV(InputStream & s, Struct & val) const
        {
            if constexpr (is_stringizable_v<Struct>)
            {
                typename Base::StructIndexType old_struct_index = ReadStructIndex(s);

                const std::vector<size_t> name_map = this->template FindProtoMap<Struct>(old_struct_index);

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
                    const DetachedPrototype & old_proto = oldPrototypes[old_struct_index];

                    assert(name_map.size() == old_proto.GetCount());

                    for (size_t old_index = 0; old_index < name_map.size(); ++old_index)
                    {
                        const Field old_field = old_proto.GetField(old_index);

                        const size_t new_index = name_map[old_index];

                        if (new_index == Prototype::NoIndex)
                        {
                            if (!this->allowDelete)
                            {
                                throw FieldNotFoundException(std::string(old_field.name));
                            }

                            SkipField(s, old_field);
                        }
                        else
                        {
                            const auto new_field = new_proto.GetField(new_index);

                            //The names are equal if a structure contains vector<A> and set<A>, for example.
                            if (!AreTypesEqual(old_field.type, new_field.type))
                            {
                                throw TypeMismatchException(std::string(new_field.name), new_field.type, old_field.type);
                            }

                            //We read by index, not by type, so we call ReadField for both structures and fields.
                            auto & readers = this->template FindFieldReaders<Struct>();
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
                Read(s, val, *this);
            }
        }

        void SkipField(InputStream & s, Field old_field) const
        {
            if (old_field.type != Field::NoType)
            {
                //Skip by type.
                auto & skippers = this->GetFieldSkippers();
                skippers[old_field.type]->SkipField(*this, s);
            }
            else
            {
                //The common routine for skipping structures.
                SkipStruct(s);
            }
        }

        void SkipStruct(InputStream & s) const
        {
            typename Base::StructIndexType old_struct_index = ReadStructIndex(s);
            const DetachedPrototype & old_proto = oldPrototypes[old_struct_index];

            for (size_t old_index = 0; old_index < old_proto.GetCount(); ++old_index)
            {
                const auto old_field = old_proto.GetField(old_index);

                SkipField(s, old_field);
            }
        }

        bool allowTypeMismatch = false;
        bool allowDelete = true;

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
            typename Base::StructIndexType index = Base::ReadStructIndex(s);
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

        template<class Struct>
        std::vector<size_t> FindProtoMap(typename Base::StructIndexType old_struct_index) const
        {
            assert(old_struct_index < oldPrototypes.size());

            constexpr size_t new_index = Base::template StructIndex<Struct>;

            if (old_struct_index < protoMaps.size())
            {
                std::optional<ProtoMap> & pm = protoMaps[old_struct_index];
                
                if (pm.has_value())
                {
                    if (new_index != pm->newStructIndex)
                    {
                        throw IoError(format() << _T("Inconsisten structure indices: new index 1: ") << pm->newStructIndex << 
                            _T(" new index 2: ") << new_index << _T(" old index: ") << old_struct_index << _T("."));
                    }

                    return pm->fieldMap;
                }
            }
            else
            {
                protoMaps.resize(old_struct_index + 1);
            }
            
            std::optional<ProtoMap> & pm = protoMaps[old_struct_index];

            pm = ProtoMap{};
            assert(pm.has_value());
            pm->newStructIndex = new_index;
            pm->fieldMap = MakeProtoMap(old_struct_index, new_index);

            return pm->fieldMap;
        }

        std::vector<size_t> MakeProtoMap(typename Base::StructIndexType old_struct_index, typename Base::StructIndexType new_struct_index) const
        {
            return MapPrototypes(oldPrototypes[old_struct_index], *(this->newPrototypes[new_struct_index]));
        }

        std::vector<size_t> MapPrototypes(const Prototype & left, const Prototype & right) const
        {
            std::vector<size_t> v;
            v.resize(left.GetCount());

            for (size_t old_index = 0; old_index < left.GetCount(); ++old_index)
            {
                v[old_index] = Prototype::NoIndex;

                const auto old_field = left.GetField(old_index);

                for (size_t new_index = 0; new_index < right.GetCount(); ++new_index)
                {
                    const auto new_field = right.GetField(new_index);

                    if (new_field.name == old_field.name)
                    {
                        //Check if the types are correct.
                        CheckTypesCompatible(old_field.type, new_field.type);

                        v[old_index] = new_index;
                        break;
                    }
                }
            }

            //If a field is added to the end of a structure the vector is trivial,
            //but the count of the fields is different.
            if (left.GetCount() == right.GetCount())
            {
                auto range = awl::make_count(v.size());
                if (std::equal(v.begin(), v.end(), range.begin(), range.end()))
                {
                    //Clear the vector if the map is trivial.
                    //If the fields are structures, we do not need to check recursively,
                    //we only guarantee that this structure is read sequentially as a tuple
                    //without readers and skippers.
                    v.clear();
                }
            }

            return v;
        }

        void CheckTypesCompatible(size_t old_type, size_t new_type) const
        {
            if (old_type != new_type)
            {
                if (old_type != Prototype::NoIndex && new_type != Prototype::NoIndex)
                {
                    //It is possible that the indices are not equal but names are.
                    if (!AreTypeNamesEqual(old_type, new_type))
                    {
                        throw IoError(_T("Type mismatch."));
                    }
                }

                if (old_type == Prototype::NoIndex || new_type == Prototype::NoIndex)
                {
                    throw IoError(_T("A structure can't map to a scalar type."));
                }
            }
        }

        bool AreTypesEqual(size_t old_type, size_t new_type) const
        {
            return old_type == new_type || AreTypeNamesEqual(old_type, new_type);
        }

        bool AreTypeNamesEqual(size_t old_type, size_t new_type) const
        {
            const std::string & old_name = typeMap[old_type];
            const std::string & new_name = typeMap[new_type];

            //The names are equal if a structure contains vector<A> and set<A>, for example.
            return old_name == new_name;
        }

        TupleOfFieldReaderTuple readerTuples;
        TupleOfFieldReaderArray readerArrays;

        SkipperTuple skipperTuple;
        SkipperArray skipperArray;

        PrototypeVector oldPrototypes;
        mutable std::vector<std::optional<ProtoMap>> protoMaps;

        typename Base::I2nMap typeMap;
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
            //Write type map
            typename Base::I2nMap tm = Base::TypeMapBuilder::BuildI2nMap();
            Write(s, tm);

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
                Write(s, val, *this);
            }
        }
    };
}
