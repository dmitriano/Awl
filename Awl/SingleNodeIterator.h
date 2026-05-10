/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/SingleList.h"

#include <cassert>
namespace awl
{
    //A transform iterator that extracts a value from a node.
    template <class Node, class Link, class T, T Node::*p_val, class Container = void>
    class single_node_iterator
    {
    private:

        using ListIterator = single_iterator<Node, Link>;
        using ConstListIterator = single_iterator<const Node, const Link>;

        using ConstNodeIterator = single_node_iterator<const Node, const Link, const T, p_val, Container>;

    public:

        using iterator_category = std::forward_iterator_tag;

        using value_type = T;

        //Required by std::iterator_traits in GCC.
        using difference_type = std::ptrdiff_t;

        using pointer = value_type *;

        using reference = value_type &;

        single_node_iterator(ListIterator i) : _i(std::move(i)) {}

        T & operator * () const
        {
            return value();
        }

        T * operator -> () const
        {
            return &value();
        }

        single_node_iterator & operator++ ()
        {
            ++_i;

            return *this;
        }

        single_node_iterator operator++ (int)
        {
            single_node_iterator tmp = *this;

            ++_i;

            return tmp;
        }

        bool operator == (const single_node_iterator & r) const
        {
            return _i == r._i;
        }

        bool operator != (const single_node_iterator & r)  const
        {
            return !(*this == r);
        }

        //Construction of const_iterator from iterator
        operator ConstNodeIterator() const
        {
            return ConstNodeIterator(ConstListIterator(_i));
        }
    
    private:

        T & value() const
        {
            Node * p_node = *_i;

            return p_node->*p_val;
        }

        ListIterator _i;

        friend Container;

        friend class VectorSetTest;
    };
}
