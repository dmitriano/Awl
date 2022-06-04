/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cassert>

#include "Awl/SingleLink.h"

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
        using Link = backward_link;

    public:

        bool included() const
        {
            return next() != nullptr;
        }

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

        template <class T1, class Link1> friend class single_iterator;
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

        bool included() const
        {
            assert(ForwardLink::included() == BackwardLink::included());
            return ForwardLink::included();
        }

        void exclude()
        {
            assert(included());
            DLink * prev = static_cast<DLink *>(this->BackwardLink::next());
            DLink * next = static_cast<DLink *>(this->ForwardLink::next());
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

        DLink * predecessor()
        {
            return static_cast<DLink *>(this->BackwardLink::next());
        }

        DLink * successor()
        {
            return static_cast<DLink *>(this->ForwardLink::next());
        }

        //Inserts a after this.
        void insert_after(DLink * a)
        {
            DLink * next = successor();
            ForwardList::insert_after(this, a);
            BackwardList::insert_after(next, a);
        }

        //Inserts a before this.
        void insert_before(DLink * a)
        {
            DLink * prev = predecessor();
            ForwardList::insert_after(prev, a);
            BackwardList::insert_after(this, a);
        }

    protected:

        basic_quick_link() {}

        //! There should not be template parameter defaults in forward declaration.
        template <class T1, class DLink1> friend class quick_list;

    public:

        basic_quick_link(basic_quick_link * next, basic_quick_link * prev) : ForwardLink(next), BackwardLink(prev)
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

    template <class DLink>
    class basic_movable_link : public basic_quick_link<DLink>
    {
    private:

        using Base = basic_quick_link<DLink>;

    public:

        using Base::Base;

        basic_movable_link(basic_movable_link&& other)
        {
            reinsert(std::move(other));
        }

        basic_movable_link& operator = (basic_movable_link&& other)
        {
            Base::safe_exclude();
            reinsert(std::move(other));
            return *this;
        }

    private:

        //Reinserts the copy into the list :)
        void reinsert(basic_movable_link&& other)
        {
            if (other.included())
            {
                DLink* prev = other.predecessor();
                other.exclude();
                prev->insert_after(static_cast<DLink*>(this));
            }
        }

        template <class T1, class DLink1> friend class quick_list;
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

#define AWL_DECLARE_MOVABLE_LINK(name) \
    class name : public awl::basic_movable_link<name> \
    { \
    private: \
        using Base = basic_movable_link<name>; \
    public:\
        using Base::Base; \
    };
