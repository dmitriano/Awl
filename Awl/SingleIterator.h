/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/SingleLink.h"

#include <iterator>

namespace awl
{
    //! The base class for list iterators. All the object in the should be of the same type T derived from Link.
    /*!	To satisfy iterator requirements, such as providing iterator_category member typedef, for example, the basic iterator derives from appropriate specialization
        of std::iterator.*/
    template <class T, class Link>
    class single_iterator
    {
    public:

        using iterator_category = std::forward_iterator_tag;

        //A value is not T but T*, because the list is a contaner of elements of type T *.
        using value_type = T*;

        //Required by std::iterator_traits in GCC.
        using difference_type = std::ptrdiff_t;

        using pointer = value_type*;

        using reference = value_type&;

        single_iterator(Link* p) : pCur(p) {}

        T* operator-> () const { return cur(); }

        T* operator* () const { return cur(); }

        single_iterator& operator++ ()
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

        bool operator == (const single_iterator& r) const
        {
            return this->link() == r.link();
        }

        bool operator != (const single_iterator& r)  const
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
        T* cur() const { return static_cast<T*>(pCur); }

        void MoveNext() { pCur = pCur->next(); }

        Link* link() const { return pCur; }

        Link* pCur;

        template <class T1, class DLink, class ForwardLink, class BackwardLink>
        friend class double_iterator;
    };
}
