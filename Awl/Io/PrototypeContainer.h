/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Prototype.h"
#include "Awl/Mp/Mp.h"
#include "Awl/TupleHelpers.h"
#include "Awl/Io/Metadata.h"
#include "Awl/Io/TypeName.h"

#include <cassert>
#include <unordered_map>
#include <vector>
#include <array>

namespace awl::io
{
    template <class V>
    class PrototypeContainer
    {
    public:

        using Variant = V;

    protected:

        using Split = mp::split_variant<V, is_reflectable>;

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

        PrototypeContainer() :
            newPrototypesTuple(transform_v2t<StructV, MyAttachedPrototype>()),
            newPrototypes(tuple_cast<Prototype>(newPrototypesTuple))
        {
        }

    public:

        //It contains the addresses of its members.
        PrototypeContainer(const PrototypeContainer&) = delete;
        PrototypeContainer(PrototypeContainer&&) = delete;
        PrototypeContainer& operator = (const PrototypeContainer&) = delete;
        PrototypeContainer& operator = (PrototypeContainer&&) = delete;

        template <class S>
        const AttachedPrototype<FieldV, S> & FindNewPrototype() const
        {
            constexpr size_t index = StructIndex<S>;
            return std::get<index>(newPrototypesTuple);
        }
    };
}
