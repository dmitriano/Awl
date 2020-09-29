#pragma once

#include <iterator>
#include <assert.h>

namespace awl
{
    //! Base class for all the single links. Objects of a class can be included into multiple lists by deriving from multiple base_single_link classes.
    /*! Link template parameter is the class pNext points to, but not this points to, but static_cast<Link *>(this) is always correct.
        Casting pNext to T * is allowed by C++ standad only if pNext points to a subobject of type T, if pNext points to Link that actually is not a subobject of T,
        the behavior of static_cast<T *>(pNext) is undefined. But casting this to T * makes sense and can be done by an iterator to access the objects it designates.
        With this type of the link we can have a list of objects of different types derived from the same type of the link, but of cause typical lists do not allow this.*/
    template <class Link>
    class base_single_link
    {
    public:

        bool included() const
        {
            return next() != nullptr;
        }

    protected:

        base_single_link(Link * n) : pNext(n) {}

        base_single_link() : pNext(nullptr) {}

        Link * next() { return pNext; }

        const Link * next() const { return pNext; }

        void set_next(Link * n)
        {
            pNext = n;
        }

    private:

        Link * pNext;

        //It can be template<class T> friend single_iterator<Link>, but C++ does not allow this.
        template <class T1, class Link1> friend class single_iterator;
        template <class T1, class Link1, class Derived1> friend class basic_single_list;
        template <class T1, class Link1> friend class single_list;
    };

    //! If objects of a class included to only one list, single_link can be used by default.
    class single_link : public base_single_link<single_link>
    {
    private:

        using Base = base_single_link<single_link>;

    public:

        using Base::Base;

        template <class T1, class Link1> friend class single_iterator;
        template <class T1, class Link1, class Derived1> friend class basic_single_list;
        template <class T1, class Link1> friend class single_list;
    };

    //! The base class for list iterators. All the object in the should be of the same type T derived from Link.
    /*!	To satisfy iterator requirements, such as providing iterator_category member typedef, for example, the basic iterator derives from appropriate specialization
        of std::iterator.*/
    template <class T, class Link>
    class single_iterator
    {
    public:

        using iterator_category = std::forward_iterator_tag;

        //A value is not T but T*, because the list is a contaner of elements of type T *.
        using value_type = T *;

        //Required by std::iterator_traits in GCC.
        using difference_type = std::ptrdiff_t;

        using pointer = value_type *;

        using reference = value_type &;

        single_iterator(Link *p) : pCur(p) {}

        T * operator-> () const { return cur(); }

        T * operator* () const { return cur(); }

        single_iterator & operator++ ()
        {
            this->MoveNext();

            return *this;
        }

        single_iterator operator++ (int)
        {
            single_iterator tmp = *this;

            this->MoveNext();

            return tmp;
        }

        bool operator == (const single_iterator & r) const
        {
            return this->link() == r.link();
        }

        bool operator != (const single_iterator & r)  const
        {
            return !(*this == r);
        }

        //Construction of const_iterator from iterator
        operator single_iterator<const T, const Link>() const
        {
            return single_iterator<const T, const Link>(link());
        }
    
    private:

        //! Results in undefined behavior if the iterator is end().
        T * cur() const { return static_cast<T *>(pCur); }

        void MoveNext() { pCur = pCur->next(); }

        Link * link() const { return pCur; }

        Link * pCur;
    };

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
