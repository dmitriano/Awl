/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "SingleList.h"
#include <cassert>

namespace awl
{
    template <class T, class DLink, class ForwardLink, class BackwardLink>
    class double_iterator
    {
    private:

        using ConstIterator = double_iterator<const T, const DLink, const ForwardLink, const BackwardLink>;
        using ReverseIterator = double_iterator<T, DLink, BackwardLink, ForwardLink>;

    public:

        using iterator_category = std::bidirectional_iterator_tag;

        //A value is not T but T*, because the list is a contaner of elements of type T *.
        using value_type = T *;

        //Required by std::iterator_traits in GCC.
        using difference_type = std::ptrdiff_t;

        using pointer = value_type *;

        using reference = value_type &;

        double_iterator() : m_p(nullptr) {}
        
        double_iterator(DLink * p) : m_p(p) {}

        //Casting the iterator with static_cast<DLink*>(*i) is UB if i == end().
        double_iterator(single_iterator<T, ForwardLink> i) : double_iterator(static_cast<DLink*>(i.link())) {}

        double_iterator(single_iterator<T, BackwardLink> i) : double_iterator(static_cast<DLink*>(i.link())) {}

        T * operator-> () const { return cur(); }

        T * operator* () const { return cur(); }

        double_iterator & operator++ ()
        {
            this->move_next();

            return *this;
        }

        double_iterator operator++ (int)
        {
            double_iterator tmp = *this;

            this->move_next();

            return tmp;
        }

        double_iterator & operator-- ()
        {
            this->move_prev();

            return *this;
        }

        double_iterator operator-- (int)
        {
            double_iterator tmp = *this;

            this->move_prev();

            return tmp;
        }

        bool operator == (const double_iterator & r) const
        {
            return this->m_p == r.m_p;
        }

        //bool operator != (const double_iterator & r)  const
        //{
        //    return !(*this == r);
        //}

        //Construction of const_iterator from iterator
        operator ConstIterator () const
        {
            return ConstIterator(m_p);
        }

        //Should I support this?
        //operator ReverseIterator () const
        //{
        //    return ReverseIterator(m_p);
        //}
    
    private:

        T * cur() const
        {
            return static_cast<T *>(m_p);
        }

        void move_next()
        {
            ForwardLink * p_link = static_cast<ForwardLink *>(m_p);

            ForwardLink * p_next = p_link->next();

            m_p = static_cast<DLink *>(p_next);
        }

        void move_prev()
        {
            BackwardLink * p_link = static_cast<BackwardLink *>(m_p);

            BackwardLink * p_next = p_link->next();

            m_p = static_cast<DLink *>(p_next);
        }

        DLink * m_p;
    };
}
