#pragma once

#include "Awl/ObservableSet.h"
#include "Awl/KeyCompare.h"
#include "Awl/TypeTraits.h"
#include "Awl/FunctionTraits.h"

#include <cassert>

namespace awl
{
    template <class T, class PrimaryKeyGetter, class ForeignKeyGetter>
    class foreign_set : public Observer<INotifySetChanged<T>>
    {
    private:

        //Plain pointer and std::shared_ptr<A> -> themselves
        //std::unique_ptr<A> -> A *
        //another type A -> A *
        using Pointer = std::conditional_t<is_copyable_pointer_v<T>, T, const remove_pointer_t<T>*>;

        using ForeignKey = typename function_traits<ForeignKeyGetter>::result_type;
        using PrimaryCompare = KeyCompare<Pointer, PrimaryKeyGetter>;
        using ValueSet = observable_set<Pointer, PrimaryCompare>;

        class ValueSetCompare
        {
        public:

            bool operator()(const ValueSet & left, const ValueSet & right) const
            {
                return GetForeignKey(left) < GetForeignKey(right);
            }

            constexpr bool operator()(const ValueSet& val, const ForeignKey & id) const
            {
                return GetForeignKey(val) < id;
            }

            constexpr bool operator()(const ForeignKey & id, const ValueSet& val) const
            {
                return id < GetForeignKey(val);
            }

        private:

            ForeignKey GetForeignKey(const ValueSet & vs) const
            {
                assert(!vs.empty());
                return foreignKeyGetter(*vs.front());
            }

            ForeignKeyGetter foreignKeyGetter;
        };
        
        using MultiSet = observable_set<ValueSet, ValueSetCompare>;
        using MultiSetObserver = Observer<INotifySetChanged<ValueSet>>;

    public:

        foreign_set(PrimaryKeyGetter pk_getter = {}, ForeignKeyGetter fk_getter = {}) :
            primaryKeyGetter(pk_getter), foreignKeyGetter(fk_getter)
        {
        }

        using value_type = ValueSet;

        using size_type = typename MultiSet::size_type;
        using difference_type = typename MultiSet::difference_type;
        using const_reference = typename MultiSet::const_reference;

        using const_iterator = typename MultiSet::const_iterator;
        using const_reverse_iterator = typename MultiSet::const_reverse_iterator;

        using allocator_type = typename MultiSet::allocator_type;
        using key_compare = typename MultiSet::key_compare;
        using value_compare = typename MultiSet::value_compare;

        const ValueSet & front() const { return m_set.front(); }
        const ValueSet & back() const { return m_set.back(); }

        const_iterator begin() const { return m_set.begin(); }
        const_iterator end() const { return m_set.end(); }
        const_reverse_iterator rbegin() const { return m_set.rbegin(); }
        const_reverse_iterator rend() const { return m_set.rend(); }

        bool empty() const
        {
            return m_set.empty();
        }

        size_type size() const
        {
            return m_set.size();
        }

        const_reference operator[](size_type pos) const
        {
            return m_set[pos];
        }

        const_reference at(size_type pos) const
        {
            return m_set.at(pos);
        }

        template <class Key>
        size_type index_of(const Key & key) const
        {
            return m_set.index_of(key);
        }

        template <class Key>
        const_iterator find(const Key & key) const
        {
            return m_set.find(key);
        }

        void Subscribe(MultiSetObserver* p_observer) const
        {
            m_set.Subscribe(p_observer);
        }

        void Unsubscribe(MultiSetObserver* p_observer) const
        {
            m_set.Unsubscribe(p_observer);
        }

    private:

        static constexpr Pointer ValueToPointer(const T& val)
        {
            if constexpr (is_copyable_pointer_v<T>)
            {
                //T
                return val;
            }
            else if constexpr (is_specialization_v<T, std::unique_ptr>)
            {
                //const remove_pointer_t<T>*
                return val.get();
            }
            else
            {
                //const T*
                return &val;
            }
        }

        void OnAdded(const T & val) override
        {
            auto& val_ref = *object_address(val);

            auto i = m_set.find(foreignKeyGetter(val_ref));

            if (i != m_set.end())
            {
                ValueSet & vs = *i;
                const bool is_new = vs.insert(ValueToPointer(val)).second;
                assert(is_new);
                static_cast<void>(is_new);
            }
            else
            {
                ValueSet vs;
                vs.insert(ValueToPointer(val));
                const bool is_new = m_set.insert(std::move(vs)).second;
                assert(is_new);
                static_cast<void>(is_new);
            }
        }

        void OnRemoving(const T & val) override
        {
            auto& val_ref = *object_address(val);
            
            auto i = m_set.find(foreignKeyGetter(val_ref));

            assert(i != m_set.end());

            ValueSet & vs = *i;

            assert(!vs.empty());

            if (vs.size() == 1)
            {
                assert(primaryKeyGetter(*vs.front()) == primaryKeyGetter(val_ref));
                
                //vs destructor will fire 'OnClearing'.
                m_set.erase(vs);
            }
            else
            {
                auto j = vs.find(primaryKeyGetter(val_ref));

                assert(j != vs.end());
                
                vs.erase(j);
            }
        }

        void OnClearing() override
        {
            m_set.clear();
        }

        MultiSet m_set;

        PrimaryKeyGetter primaryKeyGetter;
        ForeignKeyGetter foreignKeyGetter;
    };
}
