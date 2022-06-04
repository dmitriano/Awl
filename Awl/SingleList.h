/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/SingleLink.h"
#include "Awl/SingleIterator.h"

#include <cassert>

namespace awl
{
    //! A singly linked list containing elements derived from single_link<T>.
    /*! Implementation of the list is based on the idea of holding some fake "null" element of type single_link<T> that goes before the first and after the last.
        Null element takes only sizeof(T*) bytes, but not sizeof(T). */
    template <class T, class Link, class Derived>
    class basic_single_list
    {
    public:

        using value_type = T *;

        using iterator = single_iterator<T, Link>;
        using const_iterator =  single_iterator<const T, const Link>;

        basic_single_list() {}

        basic_single_list(const basic_single_list& other) = delete;

        basic_single_list(basic_single_list&& other) = delete;

        ~basic_single_list() {}

        basic_single_list& operator = (const basic_single_list& other) = delete;

        basic_single_list& operator = (basic_single_list&& other) = delete;

        //! Results in undfined behavior if the list is empty.
        T * front()
        {
            assert(!empty());
            return static_cast<T *>(first());
        }

        const T * front() const
        {
            assert(!empty());
            return static_cast<const T *>(first());
        }

        //! begin() does not cast Null.next() to T *, so it can return a valid end().
        iterator begin() { return first(); }
        const_iterator begin() const { return first(); }

        iterator end() { return null(); }
        const_iterator end() const { return null(); }

        static void insert(iterator i, T * a) { insert_after(i.prev(), a); }

        void push_front(T * a)
        {
            insert_after(null(), a);
        }

        T * pop_front()
        {
            assert(!empty());
            return static_cast<T *>(remove_after(null()));
        }

        bool empty() const { return first() == null(); }
        bool empty_or_contains_one() const { return first()->next() == null(); }
        bool contains_one() const { return !empty() && empty_or_contains_one(); }

        void clear() { null()->set_next(null()); }

        //! Returns the count of elements in the list.
        size_t size() const
        {
            size_t count = 0;

            //We do not need to do dereferencing here, so we do not use std::for_each
            //std::for_each(begin(), end(), [&count](const T *) { ++count; });

            for (const_iterator i = begin(); i != end(); ++i)
            {
                ++count;
            }

            return count;
        }

    private:

        //! The same as front but can be Null.
        Link * first() { return null()->next(); }
        const Link * first() const { return null()->next(); }

        //! One or both the parameters can be end(), so they are not T*.
        static void insert_after(Link * p, Link * a)
        {
            a->set_next(p->next());
            p->set_next(a);
        }

        //! The parameter can be end(), so it is not T*.
        static Link * remove_after(Link * p)
        {
            Link * r = p->next();
            p->set_next(r->next());
            r->set_next(nullptr);
            return r;
        }

        void attach(Link * first, Link * last)
        {
            null()->set_next(first);

            last->set_next(null());
        }

        //SingList does not know its last element so it should be provided by QuickList

        void push_front(Link * first, Link * last)
        {
            Link * old_first = this->first();

            null()->set_next(first);

            last->set_next(old_first);
        }

        void push_back(Link * first, Link * last, Link * old_last)
        {
            old_last->set_next(first);

            last->set_next(null());
        }

        Link * null() { return static_cast<Link *>(&(static_cast<Derived *>(this)->Null)); }
        const Link * null() const { return static_cast<const Link *>(&(static_cast<const Derived *>(this)->Null)); }

        //! quick_list accesses null() function.
        template <class T1, class DLink> friend class quick_list;
        template <class Dlink> friend class basic_quick_link;
        friend class quick_link;
    };

    template <class T, class Link = single_link>
    class single_list : public basic_single_list<T, Link, single_list<T, Link>>
    {
    public:

        single_list() : Null(&Null)
        {
        }

    private:

        Link Null;

        template <class T1, class Link1, class Derived1> friend class basic_single_list;
    };
}
