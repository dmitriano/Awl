/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/DoubleIterator.h"

#include <cassert>

namespace awl
{
    //A transform iterator that extracts a value from a node.
    template <class Node, class DLink, class ForwardLink, class BackwardLink, 
        class T, T Node::*p_val, class Container = void>
    class double_node_iterator
    {
    private:

        using ListIterator = double_iterator<Node, DLink, ForwardLink, BackwardLink>;
        using ConstListIterator = double_iterator<const Node, const DLink, const ForwardLink, const BackwardLink>;

        using ConstNodeIterator = double_node_iterator<const Node, const DLink, const ForwardLink, const BackwardLink, const T, p_val, Container>;

    public:

        using iterator_category = std::bidirectional_iterator_tag;

        using value_type = T;

        //Required by std::iterator_traits in GCC.
        using difference_type = std::ptrdiff_t;

        using pointer = value_type *;

        using reference = value_type &;

        double_node_iterator() = default;
        
        double_node_iterator(ListIterator i) : m_i(std::move(i)) {}

        T & operator * () const
        {
            return GetValue();
        }

        T * operator -> () const
        {
            return &GetValue();
        }

        double_node_iterator & operator++ ()
        {
            ++m_i;

            return *this;
        }

        double_node_iterator operator++ (int)
        {
            double_node_iterator tmp = *this;

            ++m_i;

            return tmp;
        }

        double_node_iterator & operator-- ()
        {
            --m_i;

            return *this;
        }

        double_node_iterator operator-- (int)
        {
            double_node_iterator tmp = *this;

            --m_i;

            return tmp;
        }

        bool operator == (const double_node_iterator & r) const
        {
            return m_i == r.m_i;
        }

        bool operator != (const double_node_iterator & r)  const
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

        friend class VectorSetTest;
    };

    //Comparison of const_iterator with iterator and vs should be supported https://stackoverflow.com/questions/35390835/is-comparison-of-const-iterator-with-iterator-well-defined
    //template <class Node, class DLink, class ForwardLink, class BackwardLink,
    //    class T, T Node::*p_val, class Container = void>
    //bool operator == (const double_node_iterator<Node, DLink, ForwardLink, BackwardLink, T, p_val, Container> & left,
    //        const double_node_iterator<const Node, const DLink, const ForwardLink, const BackwardLink, const T, p_val, Container> & right)
    //{
    //    const double_node_iterator<const Node, const DLink, const ForwardLink, const BackwardLink, const T, p_val, Container> const_left =
    //        static_cast<const double_node_iterator<const Node, const DLink, const ForwardLink, const BackwardLink, const T, p_val, Container>>(left);

    //    return left == right;
    //}
}
