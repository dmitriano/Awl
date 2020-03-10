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
        constexpr forward_link() {}
    public:
        constexpr forward_link(forward_link * n) : base_single_link<forward_link<DLink>>(n) {}
    };

    //! Backward link for doubly-linked list. DLink template paramets makes it unique in the scope of parent class T.
    /*! We can define backward_link in the same way as forward_link by inheriting from base_single_link, but in this can we will not be able to rename pNext with pPrev,
        and in the debugger we will see two pNext members of a quick_list element. */
    template <class DLink>
    class backward_link
    {
        using Link = backward_link;

    public:

        constexpr bool included() const
        {
            return next() != nullptr;
        }

    protected:

        constexpr backward_link(Link * n) : pPrev(n) {}

        constexpr backward_link() : pPrev(nullptr) {}

        constexpr Link * next() { return pPrev; }

        constexpr const Link * next() const { return pPrev; }

        constexpr void set_next(Link * n)
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
    template <class DLink>
    class basic_quick_link : public forward_link<DLink>, public backward_link<DLink>
    {
    public:

        using ForwardLink = forward_link<DLink>;
        using BackwardLink = backward_link<DLink>;

        //! The elements that are not Nulls are of type DLink, but Nulls are ForwardLink and BackwardLink.
        using ForwardList = single_list<DLink, ForwardLink>;
        using BackwardList = single_list<DLink, BackwardLink>;

        constexpr bool included() const
        {
            assert(ForwardLink::included() == BackwardLink::included());
            return ForwardLink::included();
        }

        constexpr void exclude()
        {
            assert(included());
            DLink * prev = static_cast<DLink *>(this->BackwardLink::next());
            DLink * next = static_cast<DLink *>(this->ForwardLink::next());
            ForwardList::remove_after(prev);
            BackwardList::remove_after(next);
        }

        constexpr void safe_exclude()
        {
            if (included())
            {
                exclude();
            }
        }

        constexpr DLink * predecessor()
        {
            return static_cast<DLink *>(this->BackwardLink::next());
        }

        constexpr DLink * successor()
        {
            return static_cast<DLink *>(this->ForwardLink::next());
        }

        //Inserts a after this.
        constexpr void insert_after(DLink * a)
        {
            DLink * next = successor();
            ForwardList::insert_after(this, a);
            BackwardList::insert_after(next, a);
        }

        //Inserts a before this.
        constexpr void insert_before(DLink * a)
        {
            DLink * prev = predecessor();
            ForwardList::insert_after(prev, a);
            BackwardList::insert_after(this, a);
        }

    protected:

        constexpr basic_quick_link() {}

        //! There should not be template parameter defaults in forward declaration.
        template <class T1, class DLink1> friend class quick_list;

    public:

        constexpr basic_quick_link(basic_quick_link * next, basic_quick_link * prev) : ForwardLink(next), BackwardLink(prev)
        {
        }
    };

    //! If objects of a class included to only one list, quick_link can be used by default.
    class quick_link : public basic_quick_link<quick_link>
    {
    private:

        using Base = basic_quick_link<quick_link>;

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

        using iterator = typename ForwardList::iterator;
        using const_iterator = typename ForwardList::const_iterator;

        using reverse_iterator = typename BackwardList::iterator;
        using const_reverse_iterator = typename BackwardList::const_iterator;

        //This also works, but I am not sure it is correct : Null(forward().null(), backward().null())
        constexpr quick_list() : Null(&Null, &Null)
        {
        }

        constexpr quick_list(const quick_list& other) = delete;

        constexpr quick_list(quick_list&& other)
        {
            attach(other);
        }

        constexpr quick_list& operator = (const quick_list& other) = delete;

        constexpr quick_list& operator = (quick_list&& other)
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
        constexpr T * front() { return forward().front(); }
        constexpr const T * front() const { return forward().front(); }

        constexpr T * back() { return backward().front(); }
        constexpr const T * back() const { return backward().front(); }

        constexpr iterator begin() { return forward().begin(); }
        constexpr const_iterator begin() const { return forward().begin(); }

        constexpr iterator end() { return forward().end(); }
        constexpr const_iterator end() const { return forward().end(); }

        constexpr reverse_iterator rbegin() { return backward().begin(); }
        constexpr const_reverse_iterator rbegin() const { return backward().begin(); }

        constexpr reverse_iterator rend() { return backward().end(); }
        constexpr const_reverse_iterator rend() const { return backward().end(); }

        constexpr bool empty() const { return forward().empty(); }
        constexpr bool empty_or_contains_one() const { return forward().empty_or_contains_one(); }
        constexpr bool contains_one() const { return forward().contains_one(); }

        static constexpr void insert(iterator i, T * a) { insert_after(*i, a); }
        static constexpr void erase(iterator i) { remove(*i); }

        static_assert(!std::is_same<iterator, reverse_iterator>::value, "iterator and reverse_iterator are the same types.");

        static constexpr void insert(reverse_iterator i, T * a) { insert_before(*i, a); }
        static constexpr void erase(reverse_iterator i) { remove(*i); }

        constexpr void push_front(T * a) { insert_after(static_cast<DLink *>(forward().null()), a); }
        constexpr void push_back(T * a) { insert_before(static_cast<DLink *>(forward().null()), a); }

        constexpr T * pop_front() { return remove(forward().front()); }
        constexpr T * pop_back() { return remove(backward().front()); }

        constexpr void push_front(quick_list & src)
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

        constexpr void push_back(quick_list & src)
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

        constexpr void clear()
        {
            forward().clear();
            backward().clear();
        }

        constexpr size_t size() const
        {
            return forward().size();
        }

    private:

        constexpr DLink * first() { return static_cast<DLink *>(forward().first()); }
        constexpr const DLink * first() const { return static_cast<const DLink *>(forward().first()); }

        constexpr DLink * last() { return static_cast<DLink *>(backward().first()); }
        constexpr const DLink * last() const { return static_cast<const DLink *>(backward().first()); }

        //If T is included into multiple lists there can be multiple insert_after in T,
        //so we cast T to DLink first.
        static constexpr void insert_after(DLink * p, DLink * a) { p->insert_after(a); }
        static constexpr void insert_before(DLink * p, DLink * a) { p->insert_before(a); }

        constexpr void attach(quick_list & src)
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

        constexpr void attach(T * first, T * last)
        {
            forward().attach(first, last);
            backward().attach(last, first);
        }

        //! Excludes specified element from the list.
        static constexpr T * remove(T * a)
        {
            a->DLink::exclude();
            return a;
        }

        DLink Null;

        template <class T1, class Link1, class Derived1> friend class basic_single_list;
    };
}
