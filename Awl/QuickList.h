/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/SingleList.h"
#include "Awl/DoubleIterator.h"
#include "Awl/QuickLink.h"

#include <cassert>
#include <initializer_list>


namespace awl
{
    template <class T, class Link, class Derived>
    class forward_list : public basic_single_list<T, Link, Derived>
    {
    };

    template <class T, class Link, class Derived>
    class backward_list : public basic_single_list<T, Link, Derived>
    {
    };

    //! Doubly linked list consisting of two singly linked lists.
    template < class T, class DLink = quick_link>
    class quick_list :
        private forward_list<T, typename DLink::ForwardLink, quick_list<T, DLink>>,
        private backward_list<T, typename DLink::BackwardLink, quick_list<T, DLink>>
    {
    private:

        using ForwardLink = typename DLink::ForwardLink;
        using BackwardLink = typename DLink::BackwardLink;

        using ForwardList = forward_list<T, typename DLink::ForwardLink, quick_list<T, DLink>>;
        using BackwardList =  backward_list<T, typename DLink::BackwardLink, quick_list<T, DLink>>;

        ForwardList & forward() { return *this; }
        const ForwardList & forward() const { return *this; }

        BackwardList & backward() { return *this; }
        const BackwardList & backward() const { return *this; }

    public:

        using value_type = T *;

        using iterator = double_iterator<T, DLink, ForwardLink, BackwardLink>;
        using const_iterator = double_iterator<const T, const DLink, const ForwardLink, const BackwardLink>;

        using reverse_iterator = double_iterator<T, DLink, BackwardLink, ForwardLink>;
        using const_reverse_iterator = double_iterator<const T, const DLink, const BackwardLink, const ForwardLink>;

        //This also works, but I am not sure it is correct : Null(forward().null(), backward().null())
        quick_list() : Null(&Null, &Null)
        {
        }

        quick_list(std::initializer_list<value_type> init) : quick_list()
        {
            for (const value_type& val : init)
            {
                push_back(val);
            }
        }

        quick_list(const quick_list& other) = delete;

        quick_list(quick_list&& other) noexcept
        {
            attach(other);
        }

        quick_list& operator = (const quick_list& other) = delete;

        quick_list& operator = (quick_list&& other) noexcept
        {
            if (this != &other)
            {
                //quick_list cannot free its resources, so it is supposed to be empty
                assert(empty());
                attach(other);
            }
            return *this;
        }

        //There can be using ForwardList::front, but not BackwardList::front.
        T * front() { return forward().front(); }
        const T * front() const { return forward().front(); }

        T * back() { return backward().front(); }
        const T * back() const { return backward().front(); }

        iterator begin() { return forward().begin(); }
        const_iterator begin() const { return forward().begin(); }

        iterator end() { return forward().end(); }
        const_iterator end() const { return forward().end(); }

        reverse_iterator rbegin() { return backward().begin(); }
        const_reverse_iterator rbegin() const { return backward().begin(); }

        reverse_iterator rend() { return backward().end(); }
        const_reverse_iterator rend() const { return backward().end(); }

        bool empty() const { return forward().empty(); }
        bool empty_or_contains_one() const { return forward().empty_or_contains_one(); }
        bool contains_one() const { return forward().contains_one(); }

        static void insert(iterator i, T * a) { insert_after(*i, a); }
        static void erase(iterator i) { remove(*i); }

        static_assert(!std::is_same<iterator, reverse_iterator>::value, "iterator and reverse_iterator are the same types.");

        static void insert(reverse_iterator i, T * a) { insert_before(*i, a); }
        static void erase(reverse_iterator i) { remove(*i); }

        static void erase(T* a) { remove(a); }

        void push_front(T * a) { insert_after(static_cast<DLink *>(forward().null()), a); }
        void push_back(T * a) { insert_before(static_cast<DLink *>(forward().null()), a); }

        T * pop_front() { return remove(forward().front()); }
        T * pop_back() { return remove(backward().front()); }

        void push_front(quick_list & src)
        {
            if (!src.empty())
            {
                DLink * first = src.first();
                DLink * last = src.last();

                DLink * old_last = this->first(); //the last element in the backward list

                forward().push_front(first, last);
                backward().push_back(last, first, old_last);

                src.clear();
            }
        }

        void push_back(quick_list & src)
        {
            if (!src.empty())
            {
                DLink * first = src.first();
                DLink * last = src.last();

                DLink * old_last = this->last(); //the last element in the forward list

                forward().push_back(first, last, old_last);
                backward().push_front(last, first);

                src.clear();
            }
        }

        void clear()
        {
            forward().clear();
            backward().clear();
        }

        size_t size() const
        {
            return forward().size();
        }

    private:

        DLink * first() { return static_cast<DLink *>(forward().first()); }
        const DLink * first() const { return static_cast<const DLink *>(forward().first()); }

        DLink * last() { return static_cast<DLink *>(backward().first()); }
        const DLink * last() const { return static_cast<const DLink *>(backward().first()); }

        //If T is included into multiple lists there can be multiple insert_after in T,
        //so we cast T to DLink first.
        static void insert_after(DLink * p, DLink * a) { p->insert_after(a); }
        static void insert_before(DLink * p, DLink * a) { p->insert_before(a); }

        void attach(quick_list & src)
        {
            if (src.empty())
            {
                clear();
            }
            else
            {
                T * first = src.front();
                T * last = src.back();

                attach(first, last);

                src.clear();
            }
        }

        void attach(T * first, T * last)
        {
            forward().attach(first, last);
            backward().attach(last, first);
        }

        //! Excludes specified element from the list.
        static T * remove(T * a)
        {
            a->DLink::exclude();
            return a;
        }

        DLink Null;

        template <class T1, class Link1, class Derived1> friend class basic_single_list;
    };
}

#define AWL_DECLARE_QUICK_LINK(name) \
    class name : public awl::basic_quick_link<name> \
    { \
    private: \
        using Base = basic_quick_link<name>; \
    public:\
        using Base::Base; \
    };
