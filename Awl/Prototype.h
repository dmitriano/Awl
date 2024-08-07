/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Reflection.h"
#include "Awl/Hashable.h"

#include <vector>
#include <functional>
#include <limits>
#include <assert.h>

namespace awl
{
    struct Field
    {
        static constexpr size_t NoType = std::numeric_limits<size_t>::max();

        std::string_view name;
        size_t type;

        AWL_TUPLIZABLE(name, type)
    };

    AWL_MEMBERWISE_EQUATABLE(Field)
        
    class Prototype
    {
    public:

        virtual Field GetField(size_t index) const = 0;

        virtual size_t GetCount() const = 0;

        virtual ~Prototype() {}

        static constexpr size_t NoIndex = std::numeric_limits<size_t>::max();
    };

    namespace helpers
    {
        template <class U, class T, std::size_t index>
        constexpr std::size_t find_type_impl() noexcept
        {
            using FieldType = std::remove_reference_t<std::tuple_element_t<index, U>>;
            
            if constexpr (is_reflectable_v<FieldType>)
            {
                return Field::NoType;
            }
            else
            {
                return find_variant_type_v<FieldType, T>;
            }
        }

        template <class U, class T, std::size_t... index>
        constexpr auto map_types_t2v_no_struct_impl(std::index_sequence<index...>) noexcept
        {
            return std::array<std::size_t, sizeof...(index)>{find_type_impl<U, T, index>()...};
        }

        //U is a tuple, T is a variant
        template <class U, class T>
        constexpr auto map_types_t2v_no_struct() noexcept
        {
            return map_types_t2v_no_struct_impl<U, T>(std::make_index_sequence<std::tuple_size_v<U>>());
        }
    }

    //V is std::variant, S is a Stringizable
    template <class V, class S>
    class AttachedPrototype : public Prototype
    {
    private:

        using Tie = typename tuplizable_traits<S>::Tie;
    
    public:

        AttachedPrototype() : m_types(helpers::map_types_t2v_no_struct<Tie, V>())
        {
            assert(m_types.size() == S::get_member_names().size());
        }

        Field GetField(size_t index) const override
        {
            assert(index < GetCount());
            return { S::get_member_names()[index], m_types[index] };
        }

        size_t GetCount() const override
        {
            return m_types.size();
        }

    private:

        using TypesArray = std::array<size_t, std::tuple_size_v<Tie>>;
        TypesArray m_types;
    };

    class DetachedPrototype : public Prototype
    {
    public:

        struct FieldContainer
        {
            std::string name;
            size_t type;

            AWL_TUPLIZABLE(name, type)
        };

        DetachedPrototype() = default;
        
        explicit DetachedPrototype(std::vector<FieldContainer> fields) : m_fields(std::move(fields))
        {
        }

        explicit DetachedPrototype(const Prototype & ap)
        {
            m_fields.resize(ap.GetCount());
            for (size_t i = 0; i < ap.GetCount(); ++i)
            {
                const Field field = ap.GetField(i);
                m_fields[i] = {std::string(field.name), field.type};
            }
        }

        Field GetField(size_t index) const override
        {
            assert(index < GetCount());
            const FieldContainer & m = m_fields[index];
            return { m.name, m.type };
        }

        size_t GetCount() const override
        {
            return m_fields.size();
        }

        //For type map
        void SetFieldType(size_t index, size_t type)
        {
            assert(index < GetCount());
            assert(type != Field::NoType);
            assert(m_fields[index].type != Field::NoType);

            m_fields[index].type = type;
        }

        AWL_TUPLIZABLE(m_fields)

    private:

        std::vector<FieldContainer> m_fields;
    };

    AWL_MEMBERWISE_EQUATABLE(DetachedPrototype::FieldContainer)
    AWL_MEMBERWISE_EQUATABLE(DetachedPrototype)
}
