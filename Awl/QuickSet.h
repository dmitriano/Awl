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
            T, &Node::_val, quick_set>;

        using const_iterator = double_node_iterator<const Node, const quick_link, const typename quick_link::ForwardLink, const typename quick_link::BackwardLink,
            const T, &Node::_val, quick_set>;

        using reverse_iterator = double_node_iterator<Node, quick_link, typename quick_link::BackwardLink, typename quick_link::ForwardLink,
            T, &Node::_val, quick_set>;

        using const_reverse_iterator = double_node_iterator<const Node, const quick_link, const typename quick_link::BackwardLink, const typename quick_link::ForwardLink,
            const T, &Node::_val, quick_set>;

        using key_compare = Compare;
        using value_compare = Compare;

        quick_set() : _tree(Compare{}) {}

        quick_set(Compare comp) : _tree(comp) {}

        quick_set(const quick_set& other) = delete;

        quick_set(quick_set&& other) noexcept : _tree(std::move(other._tree))
        {
            other._tree._root = nullptr;
        }

        ~quick_set()
        {
            clear();
        }

        quick_set& operator = (const quick_set& other) = delete;

        quick_set& operator = (quick_set&& other) noexcept
        {
            clear();
            _tree = std::move(other._tree);
            other._tree._root = nullptr;
            return *this;
        }

        bool operator == (const quick_set& other) const
        {
            if (size() == other.size())
            {
                const_iterator i = begin();
                for (const value_type & val : other)
                {
                    if (_tree._comp(val, *i) || _tree._comp(*i, val))
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

        T & front() { return _tree._list.front()->_val; }
        const T & front() const { return _tree._list.front()->_val; }

        T & back() { return _tree._list.back()->_val; }
        const T & back() const { return _tree._list.back()->_val; }

        iterator begin() { return _tree._list.begin(); }
        const_iterator begin() const { return _tree._list.begin(); }

        iterator end() { return _tree._list.end(); }
        const_iterator end() const { return _tree._list.end(); }

        reverse_iterator rbegin() { return _tree._list.rbegin(); }
        const_reverse_iterator rbegin() const { return _tree._list.rbegin(); }

        reverse_iterator rend() { return _tree._list.rend(); }
        const_reverse_iterator rend() const { return _tree._list.rend(); }

        std::pair<iterator, bool> insert(Node* node)
        {
            return universalInsert(node);
        }

        bool empty() const
        {
            return _tree.empty();
        }

        size_type size() const
        {
            return _tree.size();
        }

        template <class Key>
        const_iterator find(const Key& key) const
        {
            return nodeToConstIterator(_tree.findNodeByKey(key));
        }

        template <class Key>
        iterator find(const Key& key)
        {
            return nodeToIterator(_tree.findNodeByKey(key));
        }

        //calculating the index requires the iteration from the root
        //and the calculation the sum of number of the elements in the left subtrees
        //of the parent nodes.
        template <class Key>
        std::tuple<const_iterator, size_type> find2(const Key& key) const
        {
            auto [node, index] = indexByKey(key);

            return std::make_tuple(nodeToConstIterator(node), index);
        }

        template <class Key>
        std::tuple<iterator, size_type> find2(const Key& key)
        {
            auto [node, index] = _tree.indexByKey(key);

            return std::make_tuple(nodeToIterator(node), index);
        }

        //With size() and greater it returns end().
        const_iterator find_by_index(size_type pos) const
        {
            return nodeToConstIterator(_tree.findNodeByIndex(pos));
        }

        iterator find_by_index(size_type pos)
        {
            return nodeToIterator(_tree.findNodeByIndex(pos));
        }

        template <class Key>
        bool contains(const Key& key) const
        {
            return _tree.findNodeByKey(key) != nullptr;
        }

        template <class Key>
        const_iterator lower_bound(const Key& key) const
        {
            return nodeToConstIterator(std::get<0>(_tree.boundByKey(key)));
        }

        template <class Key>
        iterator lower_bound(const Key& key)
        {
            return nodeToIterator(std::get<0>(_tree.boundByKey(key)));
        }

        template <class Key>
        const_iterator upper_bound(const Key& key) const
        {
            auto [node, equal] = _tree.boundByKey(key);

            if (equal)
            {
                //return its next
                return const_iterator(++typename List::const_iterator(node));
            }

            return nodeToConstIterator(node);
        }

        template <class Key>
        iterator upper_bound(const Key& key)
        {
            auto [node, equal] = _tree.boundByKey(key);

            if (equal)
            {
                //return its next
                return iterator(++typename List::iterator(node));
            }

            return nodeToIterator(node);
        }

        reference operator[](size_type pos)
        {
            return _tree.findNodeByIndex(pos)->_val;
        }

        const_reference operator[](size_type pos) const
        {
            return _tree.findNodeByIndex(pos)->_val;
        }

        reference at(size_type pos)
        {
            checkPosition(pos);
            return (*this)[pos];
        }

        const_reference at(size_type pos) const
        {
            checkPosition(pos);
            return (*this)[pos];
        }

        //The iterator can't hold the element index because in this case
        //an insertion or deletion will invalidate it.
        size_type index_of(iterator i) const
        {
            return _tree.indexOfNode(*i._i);
        }

        size_type index_of(const_iterator i) const
        {
            return _tree.indexOfNode(*i._i);
        }

        template <class Key>
        size_type index_of(const Key& key) const
        {
            auto [node, index] = _tree.indexByKey(key);

            if (node == nullptr)
            {
                throw std::out_of_range("Key not found.");
            }

            return index;
        }

        //TODO: It should return an iterator pointing to the next element.
        void erase(iterator i)
        {
            Node* z = *i._i;

            _tree.removeNode(z);
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
            _tree._root = nullptr;
        }

        auto value_comp() const
        {
            return _tree._comp;
        }

        //Not quite correct - it should compare keys, but not values.
        auto key_comp() const
        {
            return _tree._comp;
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

        const_iterator nodeToConstIterator(const Node* node) const
        {
            if (node != nullptr)
            {
                return iterator_from_address(node);
            }

            return end();
        }

        iterator nodeToIterator(Node* node)
        {
            if (node != nullptr)
            {
                return iterator_from_address(node);
            }

            return end();
        }

        std::pair<iterator, bool> universalInsert(Node* node)
        {
            Node * parent;
            Node * existing_node = _tree.findNodeByKey(node->value(), &parent);
            const bool exists = existing_node != nullptr;

            if (!exists)
            {
                _tree.insertNode(node, parent);
            }

            return std::make_pair(iterator(typename List::iterator(node)), !exists);
        }

        void checkPosition(size_type pos) const
        {
            if (!(pos < size()))
            {
                throw std::out_of_range(std::format("Index {} is out of range [0, {}].", pos, size()));
            }
        }

        helpers::RedBlackTree<Node, T, Compare> _tree;
    };
}
