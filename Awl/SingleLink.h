/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cassert>

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

        base_single_link(Link * n) : pNext(n) {}

        base_single_link() : pNext(nullptr) {}

        base_single_link(const base_single_link& other) = delete;

        base_single_link& operator = (const base_single_link& other) = delete;

        base_single_link(base_single_link&& other) = delete;

        base_single_link& operator = (base_single_link&& other) = delete;

        ~base_single_link() = default;

        Link * next() { return pNext; }

        const Link * next() const { return pNext; }

        void set_next(Link * n)
        {
            pNext = n;
        }

    private:

        Link * pNext;
    };

    //! If objects of a class included to only one list, single_link can be used by default.
    class single_link : public base_single_link<single_link>
    {
    private:

        using Base = base_single_link<single_link>;

    public:

        using Base::Base;
    };
}
