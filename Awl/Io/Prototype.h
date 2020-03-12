#pragma once

#include "Awl/Stringizable.h"
#include "Awl/Io/TypeHash.h"

#include <vector>
#include <functional>
#include <limits>
#include <cassert>

namespace awl::io
{
    struct Field
    {
        TypeId type;
        std::string_view typeName;
        std::string_view fieldName;

        AWL_SERIALIZABLE(type, fieldName)
    };

    AWL_MEMBERWISE_EQUATABLE(Field)
        
    class Prototype
    {
    public:

        virtual Field GetField(size_t index) const = 0;

        virtual size_t GetCount() const = 0;

        static constexpr size_t NoIndex = std::numeric_limits<size_t>::max();
        
        std::vector<size_t> MapNames(const Prototype & other) const
        {
            std::vector<size_t> v;
            v.resize(GetCount());

            for (size_t old_index = 0; old_index < GetCount(); ++old_index)
            {
                v[old_index] = NoIndex;
                
                const auto old_field = GetField(old_index);

                for (size_t new_index = 0; new_index < other.GetCount(); ++new_index)
                {
                    const auto new_field = other.GetField(new_index);

                    if (new_field.fieldName == old_field.fieldName)
                    {
                        v[old_index] = new_index;
                        break;
                    }
                }
            }

            return v;
        }
    };

    namespace helpers
    {
        template <class Tuple, std::size_t... index>
        constexpr auto MakeTypeNames(std::index_sequence<index...>)
        {
            return std::make_tuple(make_type_name<std::remove_reference_t<std::tuple_element_t<index, Tuple>>>() ...);
        }

        template <class Tuple>
        constexpr auto MakeTypeNames()
        {
            return MakeTypeNames<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple>>());
        }
    }

    template <class S>
    class AttachedPrototype : public Prototype
    {
    private:

        using Tie = typename tuplizable_traits<S>::Tie;
        using TypeNamesTuple = decltype(helpers::MakeTypeNames<Tie>());
        static constexpr size_t m_size = std::tuple_size_v<Tie>;

    public:

        AttachedPrototype() : 
            typeNameTuple(helpers::MakeTypeNames<Tie>()),
            typeNames(tuple_to_array(typeNameTuple, [](const auto & name) { return std::string_view(name.data(), name.size()); })),
            typeHashes(tuple_to_array(typeNameTuple, [](const auto & name) { return calc_type_hash(name); }))
        {
            assert(typeHashes.size() == S::get_member_names().size());
        }

        Field GetField(size_t index) const override
        {
            assert(index < GetCount());
            return { typeHashes[index], typeNames[index], S::get_member_names()[index] };
        }

        size_t GetCount() const override
        {
            return typeHashes.size();
        }

    private:
        
        TypeNamesTuple typeNameTuple;
        
        using TypeNamesArray = std::array<std::string_view, m_size>;
        TypeNamesArray typeNames;

        using TypesArray = std::array<TypeId, m_size>;
        TypesArray typeHashes;
    };

    /*
    class DetachedPrototype : public Prototype
    {
    public:

        DetachedPrototype() = default;
        
        explicit DetachedPrototype(std::vector<Field> fields) : m_fields(std::move(fields))
        {
        }

        explicit DetachedPrototype(const Prototype & ap)
        {
            m_fields.resize(ap.GetCount());
            for (size_t i = 0; i < ap.GetCount(); ++i)
            {
                const FieldRef field = ap.GetField(i);
                m_fields[i] = {field.name, field.type};
            }
        }

        FieldRef GetField(size_t index) const override
        {
            assert(index < GetCount());
            const Field & m = m_fields[index];
            return { m.name, m.type };
        }

        size_t GetCount() const override
        {
            return m_fields.size();
        }

        AWL_SERIALIZABLE(m_fields)

    private:

        std::vector<Field> m_fields;
    };

    AWL_MEMBERWISE_EQUATABLE(DetachedPrototype)
    */
}
