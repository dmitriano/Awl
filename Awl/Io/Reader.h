/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/BasicReader.h"
#include "Awl/Io/FieldMap.h"
#include "Awl/Io/SequentialStream.h"
#include "Awl/Reflection.h"
#include "Awl/TupleHelpers.h"
#include "Awl/IntRange.h"
#include "Awl/Io/IoException.h"

#include <cassert>
#include <optional>
#include <vector>

namespace awl::io
{
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
            virtual void readField(const Reader & context, InputStream & in, Struct & val) const = 0;
        };

        template <class Struct, size_t index>
        class FieldReaderImpl : public FieldReader<Struct>
        {
        public:

            void readField(const Reader & context, InputStream & in, Struct & val) const override
            {
                auto & field_val = std::get<index>(val.as_tuple());

                if constexpr (reflectable<std::remove_reference_t<decltype(field_val)>>)
                {
                    context.readV(in, field_val);
                }
                else
                {
                    read(in, field_val, context);
                }
            }
        };

        struct FieldSkipper
        {
            virtual void skipField(const Reader & context, InputStream & in) const = 0;
        };

        template <class Field>
        class FieldSkipperImpl : public FieldSkipper
        {
        public:

            void skipField(const Reader & context, InputStream & in) const override
            {
                // It is the only place where Field type is required to be default constructable.
                Field val;
                context.readV(in, val);
            }
        };

        template <class Struct>
        struct FieldReaderTupleCreator
        {
            static constexpr size_t fieldCount = std::tuple_size_v<typename tuplizable_traits<Struct>::Tie>;

            template <std::size_t... index>
            static auto makeTuple(std::index_sequence<index...>)
            {
                return std::make_tuple(FieldReaderImpl<Struct, index>()...);
            }

            static auto makeTuple()
            {
                return makeTuple(std::make_index_sequence<fieldCount>());
            }
        };

        template <class Struct>
        using FieldReaderTuple = decltype(FieldReaderTupleCreator<Struct>::makeTuple());

        template <class Struct>
        using FieldReaderArray = std::array<const FieldReader<Struct> *, FieldReaderTupleCreator<Struct>::fieldCount>;

        template <size_t index>
        struct FieldReaderArrayHolder
        {
            using Struct = std::variant_alternative_t<index, typename Base::StructV>;

            FieldReaderArrayHolder(const FieldReaderTuple<Struct> & t) :
                a(tuple_cast<const FieldReader<Struct>>(t))
            {}

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
            typeMap(Base::TypeMapBuilder::buildI2nMap())
        {}

        //Makes the new and old prototypes identical.
        void initialize()
        {
            //Type map is trivial, so we do not use it.

            assert(oldPrototypes.empty());

            for (Prototype * p : this->newPrototypes)
            {
                oldPrototypes.push_back(DetachedPrototype(*p));
            }
        }

        //We can't handle the situation when type representation changes,
        //but when type name changes, type_map parameter can be used.
        //Tt can be {{"QString"}, {"sequence<int8_t>"}}, for example.
        template <class Stream>
        void readOldPrototypes(Stream& s, std::unordered_map<std::string, std::string> type_map = {})
        {
            Metadata meta;
            
            read(s, meta);

            for (std::string& type_name : meta.typeNames)
            {
                auto i = type_map.find(type_name);

                if (i != type_map.end())
                {
                    type_name = i->second;
                }
            }

            attachMetadata(meta);
        }

        void attachMetadata(Metadata & meta)
        {
            const typename Base::I2nMap old_tm = meta.typeNames;

            assert(oldPrototypes.empty());
            std::vector<DetachedPrototype> protos = meta.prototypes;

            typename Base::N2iMap new_tm = Base::TypeMapBuilder::buildN2iMap();

            for (DetachedPrototype & old_proto : protos)
            {
                for (size_t old_index = 0; old_index < old_proto.count(); ++old_index)
                {
                    const auto old_field = old_proto.field(old_index);

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

                        old_proto.setFieldType(old_index, new_type);
                    }
                }
            }

            oldPrototypes = protos;
        }

        void clearPrototypes()
        {
            oldPrototypes.clear();
            protoMaps.clear();
        }

        template<class Struct>
        void readV(InputStream & s, Struct & val) const
        {
            if constexpr (reflectable<Struct>)
            {
                typename Base::StructIndexType old_struct_index = readStructIndex(s);

                const std::vector<size_t> name_map = this->template protoMap<Struct>(old_struct_index);

                //An empty map means either an empty structure or equal prototypes
                //(the prototypes of empty structures are equal).
                if (name_map.empty())
                {
                    //Read in the same way we write it.
                    readTuplizable(s, val);
                }
                else
                {
                    auto & new_proto = this->template newPrototype<Struct>();
                    const DetachedPrototype & old_proto = oldPrototypes[old_struct_index];

                    assert(name_map.size() == old_proto.count());

                    for (size_t old_index = 0; old_index < name_map.size(); ++old_index)
                    {
                        const Field old_field = old_proto.field(old_index);

                        const size_t new_index = name_map[old_index];

                        if (new_index == Prototype::NoIndex)
                        {
                            if (!this->allowDelete)
                            {
                                throw FieldNotFoundException(std::string(old_field.name));
                            }

                            skipField(s, old_field);
                        }
                        else
                        {
                            const auto new_field = new_proto.field(new_index);

                            //The names are equal if a structure contains vector<A> and set<A>, for example.
                            if (!areTypesEqual(old_field.type, new_field.type))
                            {
                                throw TypeMismatchException(std::string(new_field.name), new_field.type, old_field.type);
                            }

                            //We read by index, not by type, so we call ReadField for both structures and fields.
                            auto & readers = this->template fieldReaders<Struct>();
                            readers[new_index]->readField(*this, s, val);
                        }
                    }
                }
            }
            else if constexpr (tuplizable<Struct>)
            {
                readTuplizable(s, val);
            }
            else
            {
                read(s, val, *this);
            }
        }

        void skipField(InputStream & s, Field old_field) const
        {
            if (old_field.type != Field::NoType)
            {
                //Skip by type.
                auto & skippers = this->fieldSkippers();
                skippers[old_field.type]->skipField(*this, s);
            }
            else
            {
                //The common routine for skipping structures.
                skipStruct(s);
            }
        }

        void skipStruct(InputStream & s) const
        {
            typename Base::StructIndexType old_struct_index = readStructIndex(s);
            const DetachedPrototype & old_proto = oldPrototypes[old_struct_index];

            for (size_t old_index = 0; old_index < old_proto.count(); ++old_index)
            {
                const auto old_field = old_proto.field(old_index);

                skipField(s, old_field);
            }
        }

        bool allowTypeMismatch = false;
        bool allowDelete = true;

    private:

        template <class S>
        auto & fieldReaders() const
        {
            constexpr size_t index = Base::template StructIndex<S>;
            auto & holder = std::get<index>(readerArrays);
            return holder.a;
        }

        const SkipperArray & fieldSkippers() const
        {
            return skipperArray;
        }

        typename Base::StructIndexType readStructIndex(InputStream & s) const
        {
            typename Base::StructIndexType index = Base::readStructIndex(s);
            assert(index < oldPrototypes.size());
            return index;
        }

        template<class Struct>
        void readTuplizable(InputStream & s, Struct & val) const
        {
            for_each(object_as_tuple(val), [this, &s](auto& field_val)
            {
                //A tuplizable structure field can be serializable.
                this->readV(s, field_val);
            });
        }

        template<class Struct>
        std::vector<size_t> protoMap(typename Base::StructIndexType old_struct_index) const
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
                        throw IoError(std::format(_T("Inconsisten structure indices: new index 1: {} new index 2: {} old index: {}."),
                            pm->newStructIndex, new_index, old_struct_index));
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
            pm->fieldMap = makeProtoMap<Struct>(old_struct_index, new_index);

            return pm->fieldMap;
        }

        template<class Struct>
        std::vector<size_t> makeProtoMap(typename Base::StructIndexType old_struct_index, typename Base::StructIndexType new_struct_index) const
        {
            return mapPrototypes<Struct>(oldPrototypes[old_struct_index], *(this->newPrototypes[new_struct_index]));
        }

        template<class Struct>
        std::vector<size_t> mapPrototypes(const Prototype & left, const Prototype & right) const
        {
            std::vector<size_t> v;
            v.resize(left.count());

            for (size_t old_index = 0; old_index < left.count(); ++old_index)
            {
                v[old_index] = Prototype::NoIndex;

                const auto old_field = left.field(old_index);

                for (size_t new_index = 0; new_index < right.count(); ++new_index)
                {
                    const auto new_field = right.field(new_index);

                    // The user renamed a field and specialized FieldMap template class.
                    const std::string_view probably_renamed = FieldMap<Struct>::newName(old_field.name);

                    if (new_field.name == probably_renamed)
                    {
                        //Check if the types are correct.
                        checkTypesCompatible(old_field.type, new_field.type);

                        v[old_index] = new_index;
                        break;
                    }
                }
            }

            //If a field is added to the end of a structure the vector is trivial,
            //but the count of the fields is different.
            if (left.count() == right.count())
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

        void checkTypesCompatible(size_t old_type, size_t new_type) const
        {
            if (old_type != new_type)
            {
                if (old_type != Prototype::NoIndex && new_type != Prototype::NoIndex)
                {
                    //It is possible that the indices are not equal but names are.
                    if (!areTypeNamesEqual(old_type, new_type))
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

        bool areTypesEqual(size_t old_type, size_t new_type) const
        {
            return old_type == new_type || areTypeNamesEqual(old_type, new_type);
        }

        bool areTypeNamesEqual(size_t old_type, size_t new_type) const
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
}
