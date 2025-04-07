/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/DoubleNodeIterator.h"
#include "Awl/Exception.h"
#include "Awl/StringFormat.h"
#include "Awl/RedBlackTree.h"

#include <iterator>
#include <memory>
#include <initializer_list>
#include <tuple>
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <algorithm>

namespace awl
{
    template <class Node, class Compare = std::less<>> 
    class quick_set
    {
    private:

        using T = typename Node::value_type;

        using List = quick_list<Node>;

    public:

        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type & ;
        using const_reference = const value_type &;

        using iterator = double_node_iterator<Node, quick_link, typename quick_link::ForwardLink, typename quick_link::BackwardLink,
            T, &Node::m_val, quick_set>;

        using const_iterator = double_node_iterator<const Node, const quick_link, const typename quick_link::ForwardLink, const typename quick_link::BackwardLink,
            const T, &Node::m_val, quick_set>;

        using reverse_iterator = double_node_iterator<Node, quick_link, typename quick_link::BackwardLink, typename quick_link::ForwardLink,
            T, &Node::m_val, quick_set>;

        using const_reverse_iterator = double_node_iterator<const Node, const quick_link, const typename quick_link::BackwardLink, const typename quick_link::ForwardLink,
            const T, &Node::m_val, quick_set>;

        using key_compare = Compare;
        using value_compare = Compare;

        quick_set() : m_tree(Compare{}) {}

        quick_set(Compare comp) : m_tree(comp) {}

        quick_set(const quick_set& other) = delete;

        quick_set(quick_set&& other) noexcept : m_tree(std::move(other.m_tree))
        {
            other.m_tree.m_root = nullptr;
        }

        ~quick_set()
        {
            clear();
        }

        quick_set& operator = (const quick_set& other) = delete;

        quick_set& operator = (quick_set&& other) noexcept
        {
            clear();
            m_tree = std::move(other.m_tree);
            other.m_tree.m_root = nullptr;
            return *this;
        }

        bool operator == (const quick_set& other) const
        {
            if (size() == other.size())
            {
                const_iterator i = begin();
                for (const value_type & val : other)
                {
                    if (m_tree.m_comp(val, *i) || m_tree.m_comp(*i, val))
                    {
                        return false;
                    }

                    ++i;
                }

                return true;
            }

            return false;
        }

        bool operator != (const quick_set & other) const
        {
            return !operator == (other);
        }

        T & front() { return m_tree.m_list.front()->m_val; }
        const T & front() const { return m_tree.m_list.front()->m_val; }

        T & back() { return m_tree.m_list.back()->m_val; }
        const T & back() const { return m_tree.m_list.back()->m_val; }

        iterator begin() { return m_tree.m_list.begin(); }
        const_iterator begin() const { return m_tree.m_list.begin(); }

        iterator end() { return m_tree.m_list.end(); }
        const_iterator end() const { return m_tree.m_list.end(); }

        reverse_iterator rbegin() { return m_tree.m_list.rbegin(); }
        const_reverse_iterator rbegin() const { return m_tree.m_list.rbegin(); }

        reverse_iterator rend() { return m_tree.m_list.rend(); }
        const_reverse_iterator rend() const { return m_tree.m_list.rend(); }

        std::pair<iterator, bool> insert(Node* node)
        {
            return UniversalInsert(node);
        }

        bool empty() const
        {
            return m_tree.empty();
        }

        size_type size() const
        {
            return m_tree.size();
        }

        template <class Key>
        const_iterator find(const Key& key) const
        {
            return NodeToConstIterator(m_tree.FindNodeByKey(key));
        }

        template <class Key>
        iterator find(const Key& key)
        {
            return NodeToIterator(m_tree.FindNodeByKey(key));
        }

        //Calculating the index requires the iteration from the root
        //and the calculation the sum of number of the elements in the left subtrees
        //of the parent nodes.
        template <class Key>
        std::tuple<const_iterator, size_type> find2(const Key& key) const
        {
            auto [node, index] = FindIndexByKey(key);

            return std::make_tuple(NodeToConstIterator(node), index);
        }

        template <class Key>
        std::tuple<iterator, size_type> find2(const Key& key)
        {
            auto [node, index] = m_tree.FindIndexByKey(key);

            return std::make_tuple(NodeToIterator(node), index);
        }

        //With size() and greater it returns end().
        const_iterator find_by_index(size_type pos) const
        {
            return NodeToConstIterator(m_tree.FindNodeByIndex(pos));
        }

        iterator find_by_index(size_type pos)
        {
            return NodeToIterator(m_tree.FindNodeByIndex(pos));
        }

        template <class Key>
        bool contains(const Key& key) const
        {
            return m_tree.FindNodeByKey(key) != nullptr;
        }

        template <class Key>
        const_iterator lower_bound(const Key& key) const
        {
            return NodeToConstIterator(std::get<0>(m_tree.FindBoundByKey(key)));
        }

        template <class Key>
        iterator lower_bound(const Key& key)
        {
            return NodeToIterator(std::get<0>(m_tree.FindBoundByKey(key)));
        }

        template <class Key>
        const_iterator upper_bound(const Key& key) const
        {
            auto [node, equal] = m_tree.FindBoundByKey(key);

            if (equal)
            {
                //return its next
                return const_iterator(++typename List::const_iterator(node));
            }

            return NodeToConstIterator(node);
        }

        template <class Key>
        iterator upper_bound(const Key& key)
        {
            auto [node, equal] = m_tree.FindBoundByKey(key);

            if (equal)
            {
                //return its next
                return iterator(++typename List::iterator(node));
            }

            return NodeToIterator(node);
        }

        reference operator[](size_type pos)
        {
            return m_tree.FindNodeByIndex(pos)->m_val;
        }

        const_reference operator[](size_type pos) const
        {
            return m_tree.FindNodeByIndex(pos)->m_val;
        }

        reference at(size_type pos)
        {
            CheckPosition(pos);
            return (*this)[pos];
        }

        const_reference at(size_type pos) const
        {
            CheckPosition(pos);
            return (*this)[pos];
        }

        //The iterator can't hold the element index because in this case
        //an insertion or deletion will invalidate it.
        size_type index_of(iterator i) const
        {
            return m_tree.IndexOfNode(*i.m_i);
        }

        size_type index_of(const_iterator i) const
        {
            return m_tree.IndexOfNode(*i.m_i);
        }

        template <class Key>
        size_type index_of(const Key& key) const
        {
            auto [node, index] = m_tree.FindIndexByKey(key);

            if (node == nullptr)
            {
                throw std::out_of_range("Key not found.");
            }

            return index;
        }

        //TODO: It should return an iterator pointing to the next element.
        void erase(iterator i)
        {
            Node* z = *i.m_i;

            m_tree.RemoveNode(z);
        }

        //Retutns the number of removed elements.
        template <class Key>
        size_type erase(const Key& key)
        {
            iterator i = find(key);

            if (i != end())
            {
                erase(i);
                return 1;
            }

            return 0;
        }

        void clear()
        {
            m_tree.m_root = nullptr;
        }

        auto value_comp() const
        {
            return m_tree.m_comp;
        }

        //Not quite correct - it should compare keys, but not values.
        auto key_comp() const
        {
            return m_tree.m_comp;
        }

        const_iterator iterator_from_address(const Node* node) const
        {
            return const_iterator(typename List::const_iterator(node));
        }

        iterator iterator_from_address(Node* node)
        {
            return iterator(typename List::iterator(node));
        }

    private:

        const_iterator NodeToConstIterator(const Node* node) const
        {
            if (node != nullptr)
            {
                return iterator_from_address(node);
            }

            return end();
        }

        iterator NodeToIterator(Node* node)
        {
            if (node != nullptr)
            {
                return iterator_from_address(node);
            }

            return end();
        }

        std::pair<iterator, bool> UniversalInsert(Node* node)
        {
            Node * parent;
            Node * existing_node = m_tree.FindNodeByKey(node->value(), &parent);
            const bool exists = existing_node != nullptr;

            if (!exists)
            {
                m_tree.InsertNode(node, parent);
            }

            return std::make_pair(iterator(typename List::iterator(node)), !exists);
        }

        void CheckPosition(size_type pos) const
        {
            if (!(pos < size()))
            {
                throw std::out_of_range(aformat() << "Index " << pos << " is out of range [0, " << size() << "].");
            }
        }

        helpers::RedBlackTree<Node, T, Compare> m_tree;
    };
}
