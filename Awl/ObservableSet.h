/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/VectorSet.h"
#include "Awl/Observable.h"

namespace awl
{
    //The argument is const probably because it can be 'const shared_ptr<A> &'.
    template <class T>
    struct INotifySetChanged
    {
        virtual void OnAdded(const T & val) = 0;
        virtual void OnRemoving(const T & val) = 0;
        virtual void OnClearing() = 0;
    };
    
    template <class T, class Compare = std::less<>, class Allocator = std::allocator<T>> 
    class observable_set
    {
    private:

        using InternalObservable = Observable<INotifySetChanged<T>, observable_set>;
        using InternalObserver = Observer<INotifySetChanged<T>>;
        using InternalSet = vector_set<T, Compare, Allocator>;

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

        observable_set() = default;
        
        observable_set(Compare comp, const Allocator& alloc = Allocator()) : m_set(comp, alloc)
        {
        }

        //It is not clear enough what to do with the observers if we copy the set. We can leave them empty as an option.
        // observable_set(const observable_set & other) : InternalObservable{}, m_set(other.m_set)
        // {
        // }

        observable_set(const observable_set & other) = delete;

        //If it is returned from a function by value it should keep its observers and
        //do not fire a notification because the content was not changed.
        observable_set(observable_set && other) = default;

        observable_set(std::initializer_list<value_type> init, const Compare& comp = Compare(), const Allocator& alloc = Allocator()) : m_set(init, comp, alloc)
        {
        }

        observable_set(std::initializer_list<value_type> init, const Allocator& alloc)
            : observable_set(init, Compare(), alloc)
        {
        }

        observable_set & operator = (const observable_set & other) = delete;

        //Notifies that the set is clearing and removes (and clears) all the subscribers,
        //so with move assignment one set becomes another set with its content and subscribers.
        observable_set& operator = (observable_set&& other) noexcept
        {
            if (!m_set.empty())
            {
                NotifyClearing();
            }

            m_set = std::move(other.m_set);
            m_observable = std::move(other.m_observable);

            return *this;
        }

        //It does not move observers, only set elements.
        void migrate(observable_set && other)
        {
            clear();

            other.NotifyClearing();
            m_set = std::move(other.m_set);
            other.m_set.clear();

            for (const T & elem : *this)
            {
                NotifyAdded(elem);
            }
        }

        ~observable_set()
        {
            if (!m_set.empty())
            {
                NotifyClearing();
            }
        }

        bool operator == (const observable_set & other) const
        {
            return m_set == other.m_set;
        }

        bool operator != (const observable_set & other) const
        {
            return !operator == (other);
        }

        T & front() { return m_set.front(); }
        const T & front() const { return m_set.front(); }

        T & back() { return m_set.back(); }
        const T & back() const { return m_set.back(); }

        iterator begin() { return m_set.begin(); }
        const_iterator begin() const { return m_set.begin(); }

        iterator end() { return m_set.end(); }
        const_iterator end() const { return m_set.end(); }

        reverse_iterator rbegin() { return m_set.rbegin(); }
        const_reverse_iterator rbegin() const { return m_set.rbegin(); }

        reverse_iterator rend() { return m_set.rend(); }
        const_reverse_iterator rend() const { return m_set.rend(); }

        std::pair<iterator, bool> insert(const value_type & value)
        {
            const std::pair<iterator, bool> result = m_set.insert(value);
            NotifyAdded(result);
            return result;
        }

        std::pair<iterator, bool> insert(value_type && value)
        {
            const std::pair<iterator, bool> result = m_set.insert(std::move(value));
            NotifyAdded(result);
            return result;
        }

        template <class... Args>
        std::pair<iterator, bool> emplace(Args&&... args)
        {
            const std::pair<iterator, bool> result = m_set.insert(std::forward<Args>(args) ...);
            NotifyAdded(result);
            return result;
        }

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
        iterator lower_bound(const Key & key)
        {
            return m_set.lower_bound(key);
        }

        template <class Key>
        const_iterator upper_bound(const Key & key) const
        {
            return m_set.upper_bound(key);
        }

        template <class Key>
        iterator upper_bound(const Key & key)
        {
            return m_set.upper_bound(key);
        }

        reference operator[](size_type pos)
        {
            return m_set[pos];
        }

        const_reference operator[](size_type pos) const
        {
            return m_set[pos];
        }

        reference at(size_type pos)
        {
            return m_set.at(pos);
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

        //TODO: It should return an iterator pointing to the next element.
        void erase(iterator i)
        {
            NotifyRemoving(i);
            m_set.erase(i);
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
            if (!m_set.empty())
            {
                m_observable.Notify(&INotifySetChanged<T>::OnClearing);
                m_set.clear();
            }
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
            m_observable.Subscribe(p_observer);
        }

        void Unsubscribe(InternalObserver* p_observer) const
        {
            m_observable.Unsubscribe(p_observer);
        }

    private:

        void NotifyAdded(const std::pair<iterator, bool> & result)
        {
            if (result.second)
            {
                NotifyAdded(*result.first);
            }
        }

        void NotifyAdded(const T& val)
        {
            m_observable.Notify(&INotifySetChanged<T>::OnAdded, val);
        }

        void NotifyRemoving(const T & val)
        {
            m_observable.Notify(&INotifySetChanged<T>::OnRemoving, val);
        }

        void NotifyRemoving(const iterator& i)
        {
            NotifyRemoving(*i);
        }

        void NotifyClearing()
        {
            m_observable.Notify(&INotifySetChanged<T>::OnClearing);
        }

        InternalSet m_set;

        mutable InternalObservable m_observable;
    };
}
