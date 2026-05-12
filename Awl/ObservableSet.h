/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/VectorSet.h"
#include "Awl/Observable.h"

#include <type_traits>

namespace awl
{
    namespace detail
    {
        template <class Set, class = void>
        struct reverse_iterator_of
        {
            using type = typename Set::iterator;
        };

        template <class Set>
        struct reverse_iterator_of<Set, std::void_t<typename Set::reverse_iterator>>
        {
            using type = typename Set::reverse_iterator;
        };

        template <class Set, class = void>
        struct const_reverse_iterator_of
        {
            using type = typename Set::const_iterator;
        };

        template <class Set>
        struct const_reverse_iterator_of<Set, std::void_t<typename Set::const_reverse_iterator>>
        {
            using type = typename Set::const_reverse_iterator;
        };

        template <class Set, class Fallback, class = void>
        struct key_compare_of
        {
            using type = Fallback;
        };

        template <class Set, class Fallback>
        struct key_compare_of<Set, Fallback, std::void_t<typename Set::key_compare>>
        {
            using type = typename Set::key_compare;
        };

        template <class Set, class Fallback, class = void>
        struct value_compare_of
        {
            using type = Fallback;
        };

        template <class Set, class Fallback>
        struct value_compare_of<Set, Fallback, std::void_t<typename Set::value_compare>>
        {
            using type = typename Set::value_compare;
        };
    }

    //The argument is const probably because it can be 'const shared_ptr<A> &'.
    template <class T>
    struct INotifySetChanged
    {
        virtual void onAdded(const T & val) = 0;
        virtual void onRemoving(const T & val) = 0;
        virtual void onClearing() = 0;
    };
    
    template <
        class Set,
        class Compare = std::less<>,
        class Allocator = typename Set::allocator_type>
    class basic_observable_set
    {
    private:

        using This = basic_observable_set<Set, Compare, Allocator>;
        using InternalSet = Set;
        using InternalObservable = Observable<INotifySetChanged<typename InternalSet::value_type>, This>;
        using InternalObserver = Observer<INotifySetChanged<typename InternalSet::value_type>>;

    public:

        using value_type = typename InternalSet::value_type;
        using size_type = typename InternalSet::size_type;
        using difference_type = typename InternalSet::difference_type;
        using reference = typename InternalSet::reference;
        using const_reference = typename InternalSet::const_reference;

        using iterator = typename InternalSet::iterator;
        using const_iterator = typename InternalSet::const_iterator;

        using reverse_iterator = typename detail::reverse_iterator_of<InternalSet>::type;
        using const_reverse_iterator = typename detail::const_reverse_iterator_of<InternalSet>::type;

        using allocator_type = typename InternalSet::allocator_type;
        using key_compare = typename detail::key_compare_of<InternalSet, Compare>::type;
        using value_compare = typename detail::value_compare_of<InternalSet, Compare>::type;

        basic_observable_set() = default;
        
        basic_observable_set(Compare comp, const Allocator& alloc = Allocator()) : _set(comp, alloc)
        {}

        //It is not clear enough what to do with the observers if we copy the set. We can leave them empty as an option.
        // basic_observable_set(const basic_observable_set & other) : InternalObservable{}, _set(other._set)
        // {
        // }

        basic_observable_set(const basic_observable_set & other) = delete;

        //If it is returned from a function by value it should keep its observers and
        //do not fire a notification because the content was not changed.
        basic_observable_set(basic_observable_set && other) = default;

        basic_observable_set(std::initializer_list<value_type> init, const Compare& comp = Compare(), const Allocator& alloc = Allocator()) : _set(init, comp, alloc)
        {}

        basic_observable_set(std::initializer_list<value_type> init, const Allocator& alloc)
            : basic_observable_set(init, Compare(), alloc)
        {}

        basic_observable_set & operator = (const basic_observable_set & other) = delete;

        //Notifies that the set is clearing and removes (and clears) all the subscribers,
        //so with move assignment one set becomes another set with its content and subscribers.
        basic_observable_set& operator = (basic_observable_set&& other) noexcept
        {
            if (!_set.empty())
            {
                notifyClearing();
            }

            _set = std::move(other._set);
            _observable = std::move(other._observable);

            return *this;
        }

        //It does not move observers, only set elements.
        void migrate(basic_observable_set && other)
        {
            clear();

            other.notifyClearing();
            _set = std::move(other._set);
            other._set.clear();

            for (const value_type & elem : *this)
            {
                notifyAdded(elem);
            }
        }

        ~basic_observable_set()
        {
            if (!_set.empty())
            {
                notifyClearing();
            }
        }

        bool operator == (const basic_observable_set & other) const
        {
            return _set == other._set;
        }

        bool operator != (const basic_observable_set & other) const
        {
            return !operator == (other);
        }

        value_type & front() { return _set.front(); }
        const value_type & front() const { return _set.front(); }

        value_type & back() { return _set.back(); }
        const value_type & back() const { return _set.back(); }

        iterator begin() { return _set.begin(); }
        const_iterator begin() const { return _set.begin(); }

        iterator end() { return _set.end(); }
        const_iterator end() const { return _set.end(); }

        reverse_iterator rbegin() { return _set.rbegin(); }
        const_reverse_iterator rbegin() const { return _set.rbegin(); }

        reverse_iterator rend() { return _set.rend(); }
        const_reverse_iterator rend() const { return _set.rend(); }

        std::pair<iterator, bool> insert(const value_type & value)
        {
            const std::pair<iterator, bool> result = _set.insert(value);
            notifyAdded(result);
            return result;
        }

        std::pair<iterator, bool> insert(value_type && value)
        {
            const std::pair<iterator, bool> result = _set.insert(std::move(value));
            notifyAdded(result);
            return result;
        }

        template <class... Args>
        std::pair<iterator, bool> emplace(Args&&... args)
        {
            const std::pair<iterator, bool> result = _set.insert(std::forward<Args>(args) ...);
            notifyAdded(result);
            return result;
        }

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
        iterator lower_bound(const Key & key)
        {
            return _set.lower_bound(key);
        }

        template <class Key>
        const_iterator upper_bound(const Key & key) const
        {
            return _set.upper_bound(key);
        }

        template <class Key>
        iterator upper_bound(const Key & key)
        {
            return _set.upper_bound(key);
        }

        reference operator[](size_type pos)
        {
            return _set[pos];
        }

        const_reference operator[](size_type pos) const
        {
            return _set[pos];
        }

        reference at(size_type pos)
        {
            return _set.at(pos);
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

        //TODO: It should return an iterator pointing to the next element.
        void erase(iterator i)
        {
            notifyRemoving(i);
            _set.erase(i);
        }

        template <class Key>
        size_type erase(const Key & key)
        {
            iterator i = find(key);

            if (i != end())
            {
                erase(i);
                return 1;
            }

            return 0;
        }

        void clear()
        {
            if (!_set.empty())
            {
                _observable.notify(&INotifySetChanged<value_type>::onClearing);
                _set.clear();
            }
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
            _observable.subscribe(p_observer);
        }

        void unsubscribe(InternalObserver* p_observer) const
        {
            _observable.unsubscribe(p_observer);
        }

    private:

        void notifyAdded(const std::pair<iterator, bool> & result)
        {
            if (result.second)
            {
                notifyAdded(*result.first);
            }
        }

        void notifyAdded(const value_type& val)
        {
            _observable.notify(&INotifySetChanged<value_type>::onAdded, val);
        }

        void notifyRemoving(const value_type & val)
        {
            _observable.notify(&INotifySetChanged<value_type>::onRemoving, val);
        }

        void notifyRemoving(const iterator& i)
        {
            notifyRemoving(*i);
        }

        void notifyClearing()
        {
            _observable.notify(&INotifySetChanged<value_type>::onClearing);
        }

        InternalSet _set;

        mutable InternalObservable _observable;
    };

    template <class T, class Compare = std::less<>, class Allocator = std::allocator<T>>
    using observable_vector_set = basic_observable_set<vector_set<T, Compare, Allocator>, Compare, Allocator>;
}
