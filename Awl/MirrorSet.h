/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/ObservableSet.h"

namespace awl
{
    template <class T, class Compare = std::less<>, class Allocator = std::allocator<T>> 
    class mirror_set : public Observer<INotifySetChanged<T>>
    {
    private:

        using InternalObserver = Observer<INotifySetChanged<T>>;
        using InternalSet = observable_set<T, Compare, Allocator>;

    public:

        using value_type = typename InternalSet::value_type;
        using size_type = typename InternalSet::size_type;
        using difference_type = typename InternalSet::difference_type;
        using reference = typename InternalSet::reference;
        using const_reference = typename InternalSet::const_reference;

        using iterator = typename InternalSet::iterator;
        using const_iterator = typename InternalSet::const_iterator;

        using reverse_iterator = typename InternalSet::reverse_iterator;
        using const_reverse_iterator = typename InternalSet::const_reverse_iterator;

        using allocator_type = typename InternalSet::allocator_type;
        using key_compare = typename InternalSet::key_compare;
        using value_compare = typename InternalSet::value_compare;

        mirror_set() = default;
        
        mirror_set(Compare comp, const Allocator& alloc = Allocator()) : m_set(comp, alloc)
        {
        }

        mirror_set(const mirror_set& other) = delete;

        mirror_set(mirror_set&& other) = default;

        mirror_set& operator = (const mirror_set& other) = delete;

        mirror_set& operator = (mirror_set&& other) = default;

        bool operator == (const mirror_set& other) const
        {
            return m_set == other.m_set;
        }

        bool operator != (const mirror_set& other) const
        {
            return !operator == (other);
        }

        template <class SrcCompare, class SrcAllocator>
        void reflect(const observable_set<T, SrcCompare, SrcAllocator>& src_set)
        {
            for (const T& val : src_set)
            {
                OnAdded(val);
            }

            //It will unsubscribe automatically in the destructor.
            src_set.Subscribe(this);
        }

        //A mirror successfully reflects another mirror.
        template <class SrcCompare, class SrcAllocator>
        void reflect(const mirror_set<T, SrcCompare, SrcAllocator>& src_set)
        {
            for (const T& val : src_set)
            {
                OnAdded(val);
            }

            //It will unsubscribe automatically in the destructor.
            src_set.Subscribe(this);
        }

        const T & front() const { return m_set.front(); }

        const T & back() const { return m_set.back(); }

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

        template <class Key>
        iterator find(const Key & key)
        {
            return m_set.find(key);
        }

        template <class Key>
        bool contains(const Key & key) const
        {
            return m_set.contains(key);
        }

        template <class Key>
        const_iterator lower_bound(const Key & key) const
        {
            return m_set.lower_bound(key);
        }

        template <class Key>
        const_iterator upper_bound(const Key & key) const
        {
            return m_set.upper_bound(key);
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

        auto value_comp() const
        {
            return m_set.value_comp();
        }

        //Not quite correct - it should compare keys, but not values.
        auto key_comp() const
        {
            return m_set.key_comp();
        }

        allocator_type get_allocator() const
        {
            return m_set.get_allocator();
        }

        void Subscribe(InternalObserver* p_observer) const
        {
            m_set.Subscribe(p_observer);
        }

        void Unsubscribe(InternalObserver* p_observer) const
        {
            m_set.Unsubscribe(p_observer);
        }

    private:

        void OnAdded(const T& val) override
        {
            if (!m_set.insert(val).second)
            {
                throw std::logic_error("Duplicate add notification.");
            }
        }

        void OnRemoving(const T& val) override
        {
            if (m_set.erase(val) == 0)
            {
                throw std::logic_error("False remove notification.");
            }
        }

        void OnClearing() override
        {
            m_set.clear();
        }

        InternalSet m_set;
    };
}
