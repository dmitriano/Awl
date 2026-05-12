/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/INotifySetChanged.h"
#include "Awl/Observable.h"

#include <concepts>
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace awl
{
    template <
        class Set,
        class Hash = typename Set::hasher,
        class Allocator = typename Set::allocator_type>
    class basic_observable_unordered_set
    {
    private:

        using This = basic_observable_unordered_set<Set, Hash, Allocator>;
        using InternalSet = Set;
        using InternalObservable = Observable<INotifySetChanged<typename InternalSet::value_type>, This>;
        using InternalObserver = Observer<INotifySetChanged<typename InternalSet::value_type>>;

    public:

        using key_type = typename InternalSet::key_type;
        using value_type = typename InternalSet::value_type;
        using size_type = typename InternalSet::size_type;
        using difference_type = typename InternalSet::difference_type;

        using hasher = typename InternalSet::hasher;
        using key_equal = typename InternalSet::key_equal;
        using allocator_type = typename InternalSet::allocator_type;

        using reference = typename InternalSet::reference;
        using const_reference = typename InternalSet::const_reference;
        using pointer = typename InternalSet::pointer;
        using const_pointer = typename InternalSet::const_pointer;

        using iterator = typename InternalSet::iterator;
        using const_iterator = typename InternalSet::const_iterator;
        using local_iterator = typename InternalSet::local_iterator;
        using const_local_iterator = typename InternalSet::const_local_iterator;
        using node_type = typename InternalSet::node_type;
        using insert_return_type = typename InternalSet::insert_return_type;

        basic_observable_unordered_set() = default;

        explicit basic_observable_unordered_set(
            size_type bucket_count,
            const hasher& hash = hasher(),
            const key_equal& equal = key_equal(),
            const Allocator& alloc = Allocator()) :
            _set(bucket_count, hash, equal, alloc)
        {}

        basic_observable_unordered_set(size_type bucket_count, const Allocator& alloc) :
            _set(bucket_count, alloc)
        {}

        basic_observable_unordered_set(size_type bucket_count, const hasher& hash, const Allocator& alloc) :
            _set(bucket_count, hash, alloc)
        {}

        explicit basic_observable_unordered_set(const Allocator& alloc) :
            _set(alloc)
        {}

        template <class InputIt>
        basic_observable_unordered_set(
            InputIt first,
            InputIt last,
            size_type bucket_count = 0,
            const hasher& hash = hasher(),
            const key_equal& equal = key_equal(),
            const Allocator& alloc = Allocator()) :
            _set(first, last, bucket_count, hash, equal, alloc)
        {}

        template <class InputIt>
        basic_observable_unordered_set(InputIt first, InputIt last, size_type bucket_count, const Allocator& alloc) :
            _set(first, last, bucket_count, alloc)
        {}

        template <class InputIt>
        basic_observable_unordered_set(
            InputIt first,
            InputIt last,
            size_type bucket_count,
            const hasher& hash,
            const Allocator& alloc) :
            _set(first, last, bucket_count, hash, alloc)
        {}

        basic_observable_unordered_set(const basic_observable_unordered_set& other) :
            _set(other._set)
        {}

        basic_observable_unordered_set(const basic_observable_unordered_set& other, const Allocator& alloc) :
            _set(other._set, alloc)
        {}

        basic_observable_unordered_set(basic_observable_unordered_set&& other) = default;

        basic_observable_unordered_set(basic_observable_unordered_set&& other, const Allocator& alloc) :
            _set(std::move(other._set), alloc)
        {}

        basic_observable_unordered_set(
            std::initializer_list<value_type> init,
            size_type bucket_count = 0,
            const hasher& hash = hasher(),
            const key_equal& equal = key_equal(),
            const Allocator& alloc = Allocator()) :
            _set(init, bucket_count, hash, equal, alloc)
        {}

        basic_observable_unordered_set(
            std::initializer_list<value_type> init,
            size_type bucket_count,
            const Allocator& alloc) :
            _set(init, bucket_count, alloc)
        {}

        basic_observable_unordered_set(
            std::initializer_list<value_type> init,
            size_type bucket_count,
            const hasher& hash,
            const Allocator& alloc) :
            _set(init, bucket_count, hash, alloc)
        {}

        basic_observable_unordered_set& operator = (const basic_observable_unordered_set& other)
        {
            if (this != &other)
            {
                assign(other._set);
            }

            return *this;
        }

        basic_observable_unordered_set& operator = (basic_observable_unordered_set&& other) noexcept
        {
            if (!_set.empty())
            {
                notifyClearing();
            }

            _set = std::move(other._set);
            _observable = std::move(other._observable);

            return *this;
        }

        basic_observable_unordered_set& operator = (std::initializer_list<value_type> init)
        {
            assign(init);
            return *this;
        }

        void migrate(basic_observable_unordered_set&& other)
        {
            clear();

            other.notifyClearing();
            _set = std::move(other._set);
            other._set.clear();

            for (const value_type& elem : *this)
            {
                notifyAdded(elem);
            }
        }

        ~basic_observable_unordered_set()
        {
            if (!_set.empty())
            {
                notifyClearing();
            }
        }

        allocator_type get_allocator() const
        {
            return _set.get_allocator();
        }

        iterator begin() noexcept
        {
            return _set.begin();
        }

        const_iterator begin() const noexcept
        {
            return _set.begin();
        }

        const_iterator cbegin() const noexcept
        {
            return _set.cbegin();
        }

        iterator end() noexcept
        {
            return _set.end();
        }

        const_iterator end() const noexcept
        {
            return _set.end();
        }

        const_iterator cend() const noexcept
        {
            return _set.cend();
        }

        bool empty() const noexcept
        {
            return _set.empty();
        }

        size_type size() const noexcept
        {
            return _set.size();
        }

        size_type max_size() const noexcept
        {
            return _set.max_size();
        }

        void clear() noexcept
        {
            if (!_set.empty())
            {
                notifyClearing();
                _set.clear();
            }
        }

        std::pair<iterator, bool> insert(const value_type& value)
        {
            const std::pair<iterator, bool> result = _set.insert(value);
            notifyAdded(result);
            return result;
        }

        std::pair<iterator, bool> insert(value_type&& value)
        {
            const std::pair<iterator, bool> result = _set.insert(std::move(value));
            notifyAdded(result);
            return result;
        }

        iterator insert(const_iterator hint, const value_type& value)
        {
            const size_type old_size = size();
            iterator result = _set.insert(hint, value);
            notifyAddedIfSizeChanged(old_size, *result);
            return result;
        }

        iterator insert(const_iterator hint, value_type&& value)
        {
            const size_type old_size = size();
            iterator result = _set.insert(hint, std::move(value));
            notifyAddedIfSizeChanged(old_size, *result);
            return result;
        }

        template <class InputIt>
        void insert(InputIt first, InputIt last)
        {
            for (; first != last; ++first)
            {
                insert(*first);
            }
        }

        void insert(std::initializer_list<value_type> init)
        {
            insert(init.begin(), init.end());
        }

        template <class R>
        void insert_range(R&& range)
        {
            for (auto&& elem : range)
            {
                insert(std::forward<decltype(elem)>(elem));
            }
        }

        insert_return_type insert(node_type&& node)
        {
            insert_return_type result = _set.insert(std::move(node));

            if (result.inserted)
            {
                notifyAdded(*result.position);
            }

            return result;
        }

        iterator insert(const_iterator hint, node_type&& node)
        {
            const size_type old_size = size();
            iterator result = _set.insert(hint, std::move(node));
            notifyAddedIfSizeChanged(old_size, *result);
            return result;
        }

        template <class... Args>
        std::pair<iterator, bool> emplace(Args&&... args)
        {
            const std::pair<iterator, bool> result = _set.emplace(std::forward<Args>(args)...);
            notifyAdded(result);
            return result;
        }

        template <class... Args>
        iterator emplace_hint(const_iterator hint, Args&&... args)
        {
            const size_type old_size = size();
            iterator result = _set.emplace_hint(hint, std::forward<Args>(args)...);
            notifyAddedIfSizeChanged(old_size, *result);
            return result;
        }

        iterator erase(const_iterator pos)
        {
            notifyRemoving(*pos);
            return _set.erase(pos);
        }

        iterator erase(const_iterator first, const_iterator last)
        {
            while (first != last)
            {
                first = erase(first);
            }

            return _set.end();
        }

        template <class Key>
        size_type erase(const Key& key)
            requires (
                !std::same_as<std::remove_cvref_t<Key>, iterator> &&
                !std::same_as<std::remove_cvref_t<Key>, const_iterator>)
        {
            iterator i = find(key);

            if (i != end())
            {
                erase(i);
                return 1;
            }

            return 0;
        }

        void swap(basic_observable_unordered_set& other)
        {
            if (this == &other)
            {
                return;
            }

            if (!_set.empty())
            {
                notifyClearing();
            }

            if (!other._set.empty())
            {
                other.notifyClearing();
            }

            _set.swap(other._set);

            for (const value_type& elem : _set)
            {
                notifyAdded(elem);
            }

            for (const value_type& elem : other._set)
            {
                other.notifyAdded(elem);
            }
        }

        node_type extract(const_iterator pos)
        {
            notifyRemoving(*pos);
            return _set.extract(pos);
        }

        template <class Key>
        node_type extract(const Key& key)
            requires (
                !std::same_as<std::remove_cvref_t<Key>, iterator> &&
                !std::same_as<std::remove_cvref_t<Key>, const_iterator>)
        {
            iterator i = find(key);

            if (i == end())
            {
                return {};
            }

            return extract(i);
        }

        template <class H2, class P2>
        void merge(std::unordered_set<key_type, H2, P2, Allocator>& source)
        {
            AddressSet old_addresses = addresses();
            _set.merge(source);
            notifyAddedSince(old_addresses);
        }

        template <class H2, class P2>
        void merge(std::unordered_set<key_type, H2, P2, Allocator>&& source)
        {
            merge(source);
        }

        template <class H2, class P2>
        void merge(std::unordered_multiset<key_type, H2, P2, Allocator>& source)
        {
            AddressSet old_addresses = addresses();
            _set.merge(source);
            notifyAddedSince(old_addresses);
        }

        template <class H2, class P2>
        void merge(std::unordered_multiset<key_type, H2, P2, Allocator>&& source)
        {
            merge(source);
        }

        template <class SourceSet, class SourceHash, class SourceAllocator>
        void merge(basic_observable_unordered_set<SourceSet, SourceHash, SourceAllocator>& source)
        {
            std::vector<const value_type*> moving;

            for (const value_type& elem : source._set)
            {
                if (!contains(elem))
                {
                    source.notifyRemoving(elem);
                    moving.push_back(&elem);
                }
            }

            _set.merge(source._set);

            for (const value_type* p_elem : moving)
            {
                notifyAdded(*p_elem);
            }
        }

        template <class SourceSet, class SourceHash, class SourceAllocator>
        void merge(basic_observable_unordered_set<SourceSet, SourceHash, SourceAllocator>&& source)
        {
            merge(source);
        }

        template <class Key>
        size_type count(const Key& key) const
        {
            return _set.count(key);
        }

        template <class Key>
        iterator find(const Key& key)
        {
            return _set.find(key);
        }

        template <class Key>
        const_iterator find(const Key& key) const
        {
            return _set.find(key);
        }

        template <class Key>
        bool contains(const Key& key) const
        {
            return _set.contains(key);
        }

        template <class Key>
        std::pair<iterator, iterator> equal_range(const Key& key)
        {
            return _set.equal_range(key);
        }

        template <class Key>
        std::pair<const_iterator, const_iterator> equal_range(const Key& key) const
        {
            return _set.equal_range(key);
        }

        local_iterator begin(size_type n)
        {
            return _set.begin(n);
        }

        const_local_iterator begin(size_type n) const
        {
            return _set.begin(n);
        }

        const_local_iterator cbegin(size_type n) const
        {
            return _set.cbegin(n);
        }

        local_iterator end(size_type n)
        {
            return _set.end(n);
        }

        const_local_iterator end(size_type n) const
        {
            return _set.end(n);
        }

        const_local_iterator cend(size_type n) const
        {
            return _set.cend(n);
        }

        size_type bucket_count() const
        {
            return _set.bucket_count();
        }

        size_type max_bucket_count() const
        {
            return _set.max_bucket_count();
        }

        size_type bucket_size(size_type n) const
        {
            return _set.bucket_size(n);
        }

        template <class Key>
        size_type bucket(const Key& key) const
        {
            return _set.bucket(key);
        }

        float load_factor() const
        {
            return _set.load_factor();
        }

        float max_load_factor() const
        {
            return _set.max_load_factor();
        }

        void max_load_factor(float ml)
        {
            _set.max_load_factor(ml);
        }

        void rehash(size_type count)
        {
            _set.rehash(count);
        }

        void reserve(size_type count)
        {
            _set.reserve(count);
        }

        hasher hash_function() const
        {
            return _set.hash_function();
        }

        key_equal key_eq() const
        {
            return _set.key_eq();
        }

        void subscribe(InternalObserver* p_observer) const
        {
            _observable.subscribe(p_observer);
        }

        void unsubscribe(InternalObserver* p_observer) const
        {
            _observable.unsubscribe(p_observer);
        }

        bool operator == (const basic_observable_unordered_set& other) const
        {
            return _set == other._set;
        }

        bool operator != (const basic_observable_unordered_set& other) const
        {
            return !operator == (other);
        }

        friend void swap(basic_observable_unordered_set& left, basic_observable_unordered_set& right)
        {
            left.swap(right);
        }

    private:

        template <class OtherSet, class OtherHash, class OtherAllocator>
        friend class basic_observable_unordered_set;

        template <class Source>
        void assign(const Source& source)
        {
            if (!_set.empty())
            {
                notifyClearing();
            }

            _set.clear();

            for (const value_type& elem : source)
            {
                insert(elem);
            }
        }

        using AddressSet = std::unordered_set<const value_type*>;

        AddressSet addresses() const
        {
            AddressSet result;
            result.reserve(size());

            for (const value_type& elem : _set)
            {
                result.insert(&elem);
            }

            return result;
        }

        void notifyAddedSince(const AddressSet& old_addresses)
        {
            for (const value_type& elem : _set)
            {
                if (!old_addresses.contains(&elem))
                {
                    notifyAdded(elem);
                }
            }
        }

        void notifyAdded(const std::pair<iterator, bool>& result)
        {
            if (result.second)
            {
                notifyAdded(*result.first);
            }
        }

        void notifyAddedIfSizeChanged(size_type old_size, const value_type& val)
        {
            if (size() != old_size)
            {
                notifyAdded(val);
            }
        }

        void notifyAdded(const value_type& val)
        {
            _observable.notify(&INotifySetChanged<value_type>::onAdded, val);
        }

        void notifyRemoving(const value_type& val)
        {
            _observable.notify(&INotifySetChanged<value_type>::onRemoving, val);
        }

        void notifyClearing()
        {
            _observable.notify(&INotifySetChanged<value_type>::onClearing);
        }

        InternalSet _set;

        mutable InternalObservable _observable;
    };

    template <class T, class Hash = std::hash<T>, class KeyEqual = std::equal_to<T>, class Allocator = std::allocator<T>>
    using observable_unordered_set = basic_observable_unordered_set<std::unordered_set<T, Hash, KeyEqual, Allocator>, Hash, Allocator>;
}
