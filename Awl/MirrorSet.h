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
        using InternalSet = observable_vector_set<T, Compare, Allocator>;

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
        
        mirror_set(Compare comp, const Allocator& alloc = Allocator()) : _set(comp, alloc)
        {}

        mirror_set(const mirror_set& other) = delete;

        mirror_set(mirror_set&& other) = default;

        mirror_set& operator = (const mirror_set& other) = delete;

        mirror_set& operator = (mirror_set&& other) = default;

        bool operator == (const mirror_set& other) const
        {
            return _set == other._set;
        }

        bool operator != (const mirror_set& other) const
        {
            return !operator == (other);
        }

        template <class SrcCompare, class SrcAllocator>
        void reflect(const observable_vector_set<T, SrcCompare, SrcAllocator>& src_set)
        {
            for (const T& val : src_set)
            {
                onAdded(val);
            }

            //It will unsubscribe automatically in the destructor.
            src_set.subscribe(this);
        }

        //A mirror successfully reflects another mirror.
        template <class SrcCompare, class SrcAllocator>
        void reflect(const mirror_set<T, SrcCompare, SrcAllocator>& src_set)
        {
            for (const T& val : src_set)
            {
                onAdded(val);
            }

            //It will unsubscribe automatically in the destructor.
            src_set.subscribe(this);
        }

        const T & front() const { return _set.front(); }

        const T & back() const { return _set.back(); }

        const_iterator begin() const { return _set.begin(); }

        const_iterator end() const { return _set.end(); }

        const_reverse_iterator rbegin() const { return _set.rbegin(); }

        const_reverse_iterator rend() const { return _set.rend(); }

        bool empty() const
        {
            return _set.empty();
        }

        size_type size() const
        {
            return _set.size();
        }

        template <class Key>
        iterator find(const Key & key)
        {
            return _set.find(key);
        }

        template <class Key>
        bool contains(const Key & key) const
        {
            return _set.contains(key);
        }

        template <class Key>
        const_iterator lower_bound(const Key & key) const
        {
            return _set.lower_bound(key);
        }

        template <class Key>
        const_iterator upper_bound(const Key & key) const
        {
            return _set.upper_bound(key);
        }

        const_reference operator[](size_type pos) const
        {
            return _set[pos];
        }

        const_reference at(size_type pos) const
        {
            return _set.at(pos);
        }

        template <class Key>
        size_type index_of(const Key & key) const
        {
            return _set.index_of(key);
        }

        template <class Key>
        const_iterator find(const Key & key) const
        {
            return _set.find(key);
        }

        auto value_comp() const
        {
            return _set.value_comp();
        }

        //Not quite correct - it should compare keys, but not values.
        auto key_comp() const
        {
            return _set.key_comp();
        }

        allocator_type get_allocator() const
        {
            return _set.get_allocator();
        }

        void subscribe(InternalObserver* p_observer) const
        {
            _set.subscribe(p_observer);
        }

        void unsubscribe(InternalObserver* p_observer) const
        {
            _set.unsubscribe(p_observer);
        }

    private:

        void onAdded(const T& val) override
        {
            if (!_set.insert(val).second)
            {
                throw std::runtime_error("Duplicate add notification.");
            }
        }

        void onRemoving(const T& val) override
        {
            if (_set.erase(val) == 0)
            {
                throw std::runtime_error("False remove notification.");
            }
        }

        void onClearing() override
        {
            _set.clear();
        }

        InternalSet _set;
    };
}
