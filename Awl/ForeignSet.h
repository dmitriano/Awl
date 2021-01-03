#pragma once

#include "Awl/ObservableSet.h"
#include "Awl/KeyCompare.h"

#include <assert.h>

namespace awl
{
    template <class ForeignKeyGetter, class T, class PrimaryKeyGetter>
    class foreign_set : public Observer<INotifySetChanged<T>>
    {
    private:

        using ForeignKey = typename function_traits<ForeignKeyGetter>::result_type;
        using PrimaryCompare = KeyCompare<const T *, PrimaryKeyGetter>;
        using ValueSet = observable_set<const T *, PrimaryCompare>;

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

    public:

        using value_type = const ValueSet;

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

    private:

        void OnAdded(const T & val) override
        {
            auto i = m_set.find(foreignKeyGetter(val));

            if (i != m_set.end())
            {
                ValueSet & vs = *i;
                const bool is_new = vs.insert(&val).second;
                assert(is_new);
                static_cast<void>(is_new);
            }
            else
            {
                ValueSet vs;
                vs.insert(&val);
                const bool is_new = m_set.insert(std::move(vs)).second;
                assert(is_new);
                static_cast<void>(is_new);
            }
        }

        void OnRemoving(const T & val) override
        {
            auto i = m_set.find(foreignKeyGetter(val));

            assert(i != m_set.end());

            ValueSet & vs = *i;

            assert(!vs.empty());

            if (vs.size() == 1)
            {
                assert(primaryKeyGetter(*vs.front()) == primaryKeyGetter(val));
                
                //vs destructor will fire 'OnClearing'.
                m_set.erase(vs);
            }
            else
            {
                auto j = vs.find(primaryKeyGetter(val));

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
