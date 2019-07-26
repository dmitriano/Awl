#pragma once

#include <assert.h>

#include "Awl/SingleList.h"

namespace awl
{
    //! Forward link for doubly-linked list. DLink template paramets makes it unique in the scope of parent class T.
    template <class DLink>
    class forward_link : public base_single_link<forward_link<DLink>>
    {
    protected:
        forward_link() {}
    public:
        forward_link(forward_link * n) : base_single_link<forward_link<DLink>>(n) {}
    };

    //! Backward link for doubly-linked list. DLink template paramets makes it unique in the scope of parent class T.
    /*! We can define backward_link in the same way as forward_link by inheriting from base_single_link, but in this can we will not be able to rename pNext with pPrev,
        and in the debugger we will see two pNext members of a quick_list element. */
    template <class DLink>
    class backward_link
    {
        typedef backward_link Link;

    public:

        bool included() const
        {
            return next() != nullptr;
        }

    protected:

        backward_link(Link * n) : pPrev(n) {}

        backward_link() : pPrev(nullptr) {}

        Link * next() { return pPrev; }

        const Link * next() const { return pPrev; }

        void set_next(Link * n)
        {
            pPrev = n;
        }

    private:

        Link * pPrev;

        template <class T1, class Link1> friend class base_single_iterator;
        template <class T1, class Link1, class Derived1> friend class basic_single_list;
    };

    //! Double link consisting of two single links.
    /*! There the Null of type quick_link, and there are two separate singly-lined lists, that can have different offset in their enclosing class,
        but we can make forward_link::pNext and backward_link::pNext point to quick_link. Getting the address of the object by its member is illegal in C++ 17,
        so we should derive quick_link from two single links.
        basic_quick_link does not declare its own members like pNext and all the linking is actually done with the single links.
        IMPORTANT: The link sould not call safe_exclude() in its destructor, because when quick_list::clear() is called, pNull of the elements are not set to nullptr,
        so the link can be in some kind of a detached state in which pNext is not valid at all.*/
    template <class Dlink>
    class basic_quick_link : public forward_link<Dlink>, public backward_link<Dlink>
    {
    public:

        bool included() const
        {
            assert(ForwardLink::included() == BackwardLink::included());
            return ForwardLink::included();
        }

        void exclude()
        {
            assert(included());
            Dlink * prev = static_cast<Dlink *>(this->BackwardLink::next());
            Dlink * next = static_cast<Dlink *>(this->ForwardLink::next());
            ForwardList::remove_after(prev);
            BackwardList::remove_after(next);
        }

        void safe_exclude()
        {
            if (included())
            {
                exclude();
            }
        }

        typedef forward_link<Dlink> ForwardLink;
        typedef backward_link<Dlink> BackwardLink;

    protected:

        //! The elements that are not Nulls are of type Dlink, but Nulls are ForwardLink and BackwardLink.
        typedef single_list<Dlink, ForwardLink> ForwardList;
        typedef single_list<Dlink, BackwardLink> BackwardList;

        basic_quick_link() {}

        //! There should not be template parameter defaults in forward declaration.
        template <class T1, class DLink> friend class quick_list;

    public:

        basic_quick_link(basic_quick_link * next, basic_quick_link * prev) : ForwardLink(next), BackwardLink(prev)
        {
        }
    };

    //! If objects of a class included to only one list, quick_link can be used by default.
    class quick_link : public basic_quick_link<quick_link>
    {
    private:

        typedef basic_quick_link<quick_link> Base;

    public:

        using Base::Base;

        template <class T1, class DLink1> friend class quick_list;
    };


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
    class quick_list : private forward_list<T, typename DLink::ForwardLink, quick_list<T, DLink>>, private backward_list<T, typename DLink::BackwardLink, quick_list<T, DLink>>
    {
    private:

        typedef typename DLink::ForwardLink ForwardLink;
        typedef typename DLink::BackwardLink BackwardLink;

        typedef forward_list<T, typename DLink::ForwardLink, quick_list<T, DLink>> ForwardList;
        typedef backward_list<T, typename DLink::BackwardLink, quick_list<T, DLink>> BackwardList;

        ForwardList & forward() { return *this; }
        const ForwardList & forward() const { return *this; }

        BackwardList & backward() { return *this; }
        const BackwardList & backward() const { return *this; }

    public:

        typedef T * value_type;

        typedef typename ForwardList::iterator iterator;
        typedef typename ForwardList::const_iterator const_iterator;

        typedef typename BackwardList::iterator reverse_iterator;
        typedef typename BackwardList::const_iterator const_reverse_iterator;

        //This also works, but I am not sure it is correct : Null(forward().null(), backward().null())
        quick_list() : Null(&Null, &Null)
        {
        }

        quick_list(const quick_list& other) = delete;

        quick_list(quick_list&& other)
        {
            attach(other);
        }

        quick_list& operator = (const quick_list& other) = delete;

        quick_list& operator = (quick_list&& other)
        {
            if (this != &other)
            {
                //quick_list cannot free it resources, so it is supposed to be empty
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

        static void insert_after(DLink * p, DLink * a)
        {
            DLink * next = static_cast<DLink *>(p->ForwardLink::next());
            ForwardList::insert_after(p, a);
            BackwardList::insert_after(next, a);
        }

        static void insert_before(DLink * p, DLink * a)
        {
            DLink * prev = static_cast<DLink *>(p->BackwardLink::next());
            ForwardList::insert_after(prev, a);
            BackwardList::insert_after(p, a);
        }

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
