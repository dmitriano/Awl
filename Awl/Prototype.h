#pragma once

#include "Awl/Stringizable.h"

#include <vector>
#include <assert.h>

namespace awl
{
    struct Field
    {
        std::string name;
        size_t type;

        AWL_SERIALIZABLE(name, type)
    };

    AWL_MEMBERWISE_EQUATABLE(Field)
        
    struct FieldRef
    {
        const std::string & name;
        size_t type;

        AWL_SERIALIZABLE(name, type)
    };

    AWL_MEMBERWISE_EQUATABLE(FieldRef)
        
    class Prototype
    {
    public:

        virtual FieldRef GetField(size_t index) const = 0;

        virtual size_t GetCount() const = 0;
    };

    //V is std::variant, S is a Stringizable
    template <class V, class S>
    class AttachedPrototype : public Prototype
    {
    private:

        typedef typename decltype(S{}.as_tuple()) Tie;

    public:

        AttachedPrototype() : m_a(map_types_t2v<Tie, V >())
        {
            assert(m_a.size() == S::get_member_names().size());
        }

        FieldRef GetField(size_t index) const override
        {
            assert(index < GetCount());
            return { S::get_member_names()[index], m_a[index] };
        }

        size_t GetCount() const override
        {
            return m_a.size();
        }

    private:

        std::array<size_t, std::tuple_size_v<Tie>> m_a;
    };

    class DetachedPrototype : public Prototype
    {
    public:

        explicit DetachedPrototype(std::vector<Field> fields) : m_fields(std::move(fields))
        {
        }

        explicit DetachedPrototype(const Prototype & ap)
        {
            m_fields.resize(ap.GetCount());
            for (int i = 0; i < ap.GetCount(); ++i)
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
}
