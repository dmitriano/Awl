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

        constexpr bool included() const
        {
            return next() != nullptr;
        }

    protected:

        constexpr base_single_link(Link * n) : pNext(n) {}

        constexpr base_single_link() : pNext(nullptr) {}

        constexpr Link * next() { return pNext; }

        constexpr const Link * next() const { return pNext; }

        constexpr void set_next(Link * n)
        {
            pNext = n;
        }

    private:

        Link * pNext;

        //It can be template<class T> friend base_single_iterator<Link>, but C++ does not allow this.
        template <class T1, class Link1> friend class base_single_iterator;
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

        template <class T1, class Link1> friend class base_single_iterator;
        template <class T1, class Link1, class Derived1> friend class basic_single_list;
        template <class T1, class Link1> friend class single_list;
    };

    //! The base class for list iterators. All the object in the should be of the same type T derived from Link.
    /*!	To satisfy iterator requirements, such as providing iterator_category member typedef, for example, the basic iterator derives from appropriate specialization
        of std::iterator.*/
    template <class T, class Link>
    class base_single_iterator
    {
    public:

        using iterator_category = std::forward_iterator_tag;

        //A value is not T but T*, because the list is a contaner of elements of type T *.
        using value_type = T *;

        //Required by std::iterator_traits in GCC.
        using difference_type = std::ptrdiff_t;

        using pointer = value_type *;

        using reference = value_type &;

        constexpr base_single_iterator(Link *p) : pCur(p) {}

        constexpr T * operator-> () const { return cur(); }

        constexpr T * operator* () const { return cur(); }

    protected:

        //! Results in undefined behavior if the iterator is end().
        constexpr T * cur() const { return static_cast<T *>(pCur); }

        constexpr void MoveNext() { pCur = pCur->next(); }

        constexpr Link * link() const { return pCur; }

    private:

        Link * pCur;
    };

    template <class T, class Link>
    class single_iterator : public base_single_iterator<T, Link>
    {
    public:

        constexpr single_iterator(Link *p) : base_single_iterator<T, Link>(p) {}

        constexpr single_iterator(const single_iterator & other) : single_iterator(*other) {}

        constexpr single_iterator & operator = (const single_iterator &) = default;

        constexpr single_iterator & operator++ ()
        {
            this->MoveNext();

            return *this;
        }

        constexpr single_iterator operator++ (int)
        {
            single_iterator tmp = *this;

            this->MoveNext();

            return tmp;
        }

        constexpr bool operator == (const single_iterator & r) const
        {
            return this->link() == r.link();
        }

        constexpr bool operator != (const single_iterator & r)  const
        {
            return !(*this == r);
        }
    };

    template <class T, class Link>
    class const_single_iterator : public base_single_iterator<const T, const Link>
    {
    public:

        constexpr const_single_iterator(const Link *p) : base_single_iterator<const T, const Link>(p) {}

        constexpr const_single_iterator(const const_single_iterator & other) : const_single_iterator(*other) {}

        constexpr const_single_iterator & operator = (const const_single_iterator &) = default;

        //! The only differece between single_iterator and const_single_iterator is that single_iterator can be converted to const_single_iterator but not vice versa.
        constexpr const_single_iterator(const single_iterator<T, Link> & other) : const_single_iterator(*other) {}

        constexpr const_single_iterator& operator++ ()
        {
            this->MoveNext();

            return *this;
        }

        constexpr const_single_iterator operator++ (int)
        {
            const_single_iterator tmp = *this;

            this->MoveNext();

            return tmp;
        }

        constexpr bool operator == (const const_single_iterator & r) const
        {
            return this->link() == r.link();
        }

        constexpr bool operator != (const const_single_iterator & r)  const
        {
            return !(*this == r);
        }
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
        using const_iterator =  const_single_iterator<T, Link>;

        constexpr basic_single_list() {}

        constexpr basic_single_list(const basic_single_list& other) = delete;

        constexpr basic_single_list(basic_single_list&& other) = delete;

        constexpr basic_single_list& operator = (const basic_single_list& other) = delete;

        constexpr basic_single_list& operator = (basic_single_list&& other) = delete;

        //! Results in undfined behavior if the list is empty.
        constexpr T * front()
        {
            assert(!empty());
            return static_cast<T *>(first());
        }

        constexpr const T * front() const
        {
            assert(!empty());
            return static_cast<const T *>(first());
        }

        //! begin() does not cast Null.next() to T *, so it can return a valid end().
        constexpr iterator begin() { return first(); }
        constexpr const_iterator begin() const { return first(); }

        constexpr iterator end() { return null(); }
        constexpr const_iterator end() const { return null(); }

        static constexpr void insert(iterator i, T * a) { insert_after(i.prev(), a); }

        constexpr void push_front(T * a)
        {
            insert_after(null(), a);
        }

        constexpr T * pop_front()
        {
            assert(!empty());
            return static_cast<T *>(remove_after(null()));
        }

        constexpr bool empty() const { return first() == null(); }
        constexpr bool empty_or_contains_one() const { return first()->next() == null(); }
        constexpr bool contains_one() const { return !empty() && empty_or_contains_one(); }

        constexpr void clear() { null()->set_next(null()); }

        //! Returns the count of elements in the list.
        constexpr size_t size() const
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
        constexpr Link * first() { return null()->next(); }
        constexpr const Link * first() const { return null()->next(); }

        //! One or both the parameters can be end(), so they are not T*.
        static constexpr void insert_after(Link * p, Link * a)
        {
            a->set_next(p->next());
            p->set_next(a);
        }

        //! The parameter can be end(), so it is not T*.
        static constexpr Link * remove_after(Link * p)
        {
            Link * r = p->next();
            p->set_next(r->next());
            r->set_next(nullptr);
            return r;
        }

        constexpr void attach(Link * first, Link * last)
        {
            null()->set_next(first);

            last->set_next(null());
        }

        //SingList does not know its last element so it should be provided by QuickList

        constexpr void push_front(Link * first, Link * last)
        {
            Link * old_first = this->first();

            null()->set_next(first);

            last->set_next(old_first);
        }

        constexpr void push_back(Link * first, Link * last, Link * old_last)
        {
            old_last->set_next(first);

            last->set_next(null());
        }

        constexpr Link * null() { return static_cast<Link *>(&(static_cast<Derived *>(this)->Null)); }
        constexpr const Link * null() const { return static_cast<const Link *>(&(static_cast<const Derived *>(this)->Null)); }

        //! quick_list accesses null() function.
        template <class T1, class DLink> friend class quick_list;
        template <class Dlink> friend class basic_quick_link;
        friend class quick_link;
    };

    template <class T, class Link = single_link>
    class single_list : public basic_single_list<T, Link, single_list<T, Link>>
    {
    public:

        constexpr single_list() : Null(&Null)
        {
        }

    private:

        Link Null;

        template <class T1, class Link1, class Derived1> friend class basic_single_list;
    };
}
