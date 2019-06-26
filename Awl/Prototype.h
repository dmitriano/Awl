#pragma once

#include "Awl/Stringizable.h"

#include <vector>
#include <functional>
#include <limits>
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
    };
        
    class Prototype
    {
    public:

        virtual FieldRef GetField(size_t index) const = 0;

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

                    if (new_field.name == old_field.name)
                    {
                        v[old_index] = new_index;
                        break;
                    }
                }
            }

            return v;
        }
    };

    //V is std::variant, S is a Stringizable
    template <class V, class S>
    class AttachedPrototype : public Prototype
    {
    private:

        typedef typename tuplizable_traits<S>::Tie Tie;
    
    public:

        AttachedPrototype() : m_types(map_types_t2v<Tie, V>())
        {
            assert(m_types.size() == S::get_member_names().size());
        }

        FieldRef GetField(size_t index) const override
        {
            assert(index < GetCount());
            return { S::get_member_names()[index], m_types[index] };
        }

        size_t GetCount() const override
        {
            return m_types.size();
        }

        V Get(const S & val, size_t index) const
        {
            return V{};
            //return runtime_get<V>(val.as_tuple(), index);
        }

        void Set(S & val, size_t index, V v_field) const
        {
            //runtime_set(val.as_tuple(), index, v_field);
        }

    private:

        using TypesArray = std::array<size_t, std::tuple_size_v<Tie>>;
        TypesArray m_types;
    };

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
}
