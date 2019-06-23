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

            for (int old_index = 0; old_index < GetCount(); ++old_index)
            {
                v[old_index] = NoIndex;
                
                const auto old_field = GetField(old_index);

                for (int new_index = 0; new_index < other.GetCount(); ++new_index)
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

        AttachedPrototype() : m_types(map_types_t2v<Tie, V>()), m_getters(MakeGetters()), m_setters(MakeSetters())
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

        V Get(const S & val, size_t index)
        {
            return m_getters[index](val);
        }

        void Set(S & val, size_t index, V v_field)
        {
            m_setters[index](val, std::move(v_field));
        }

    private:

        typedef std::array<size_t, std::tuple_size_v<Tie>> TypesArray;
        typedef std::array<std::function<V(const S & val)>, std::tuple_size_v<Tie>> GetterArray;
        typedef std::array<std::function<void(S & val, V v_field)>, std::tuple_size_v<Tie>> SetterArray;

        auto MakeGetters() const
        {
            return MakeGetters(std::make_index_sequence<std::tuple_size_v<Tie>>());
        }

        template <std::size_t... index>
        auto MakeGetters(std::index_sequence<index...>) const
        {
            return GetterArray{
                [](const S & val) -> V
                {
                    return std::get<index>(val.as_tuple());
                }
                ...
            };
        }

        auto MakeSetters() const
        {
            return MakeSetters(std::make_index_sequence<std::tuple_size_v<Tie>>());
        }

        template <std::size_t... index>
        auto MakeSetters(std::index_sequence<index...>) const
        {
            return SetterArray{ 
                [](S & val, V && v_field)
                {
                    std::get<index>(val.as_tuple()) = std::get<std::remove_reference_t<std::tuple_element_t<index, Tie>>>(v_field);
                }
                ...
            };
        }

        TypesArray m_types;
        
        GetterArray m_getters;
        SetterArray m_setters;
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
