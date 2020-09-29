#pragma once

#include "SingleList.h"
#include <cassert>

namespace awl
{
    //A transform iterator that extracts a value from a node.
    template <class Node, class Link, class T, T Node::*p_val, class Container = void>
    class node_iterator
    {
    private:

        using ListIterator = awl::single_iterator<Node, Link>;
        using ConstListIterator = awl::single_iterator<const Node, const Link>;

        using ConstNodeIterator = node_iterator<const Node, const Link, const T, p_val, Container>;

    public:

        using iterator_category = std::forward_iterator_tag;

        using value_type = T;

        //Required by std::iterator_traits in GCC.
        using difference_type = std::ptrdiff_t;

        using pointer = value_type *;

        using reference = value_type &;

        node_iterator(ListIterator i) : m_i(std::move(i)) {}

        T & operator * () const
        {
            return GetValue();
        }

        T & operator -> () const
        {
            return GetValue();
        }

        node_iterator & operator++ ()
        {
            ++m_i;

            return *this;
        }

        node_iterator operator++ (int)
        {
            node_iterator tmp = *this;

            ++m_i;

            return tmp;
        }

        bool operator == (const node_iterator & r) const
        {
            return m_i == r.m_i;
        }

        bool operator != (const node_iterator & r)  const
        {
            return !(*this == r);
        }

        //Construction of const_iterator from iterator
        operator ConstNodeIterator() const
        {
            return ConstNodeIterator(ConstListIterator(m_i));
        }
    
    private:

        T & GetValue() const
        {
            Node * p_node = *m_i;

            return p_node->*p_val;
        }

        ListIterator m_i;

        friend Container;
    };
}
