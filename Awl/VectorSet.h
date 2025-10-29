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
    template <class T, class Compare = std::less<>, class Allocator = std::allocator<T>> 
    class vector_set
    {
    private:

        //emplace(Args...) method may construct the node even if there already is a node with the key in the container,
        //in which case the newly constructed element will be destroyed immediately. So a node is not guaranteed
        //to be included.
        struct Node : public helpers::RedBlackLink<Node>
        {
            using Link = helpers::RedBlackLink<Node>;

            Node(const T & v) : Link{}, m_val(v)
            {
            }

            Node(T && v) : Link{}, m_val(std::move(v))
            {
            }

            template <class... Args>
            Node(Args&&... args) : Link{}, m_val(std::forward<Args>(args) ...)
            {
            }

            const T& value() const
            {
                return m_val;
            }

            T m_val;
        };

        using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;

        Node * CreateNode(const T & val)
        {
            Node * node = m_nodeAlloc.allocate(1);
            new (node) Node(val);
            return node;
        }

        Node * CreateNode(T && val)
        {
            Node * node = m_nodeAlloc.allocate(1);
            new (node) Node(std::move(val));
            return node;
        }

        template <class... Args>
        Node * CreateNode(Args&&... args)
        {
            Node * node = m_nodeAlloc.allocate(1);
            new (node) Node(std::forward<Args>(args) ...);
            return node;
        }

        void DestroyNode(Node * node)
        {
            node->~Node();
            m_nodeAlloc.deallocate(node, 1);
        }

        struct NodeDeleter
        {
            vector_set * owner;

            void operator()(Node * node) const noexcept
            {
                if (node != nullptr)
                {
                    owner->DestroyNode(node);
                }
            }
        };

        using NodeHolder = std::unique_ptr<Node, NodeDeleter>;

        using List = quick_list<Node>;

    public:

        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type & ;
        using const_reference = const value_type &;

        using iterator = double_node_iterator<Node, quick_link, typename quick_link::ForwardLink, typename quick_link::BackwardLink,
            T, &Node::m_val, vector_set>;

        using const_iterator = double_node_iterator<const Node, const quick_link, const typename quick_link::ForwardLink, const typename quick_link::BackwardLink,
            const T, &Node::m_val, vector_set>;

        using reverse_iterator = double_node_iterator<Node, quick_link, typename quick_link::BackwardLink, typename quick_link::ForwardLink,
            T, &Node::m_val, vector_set>;

        using const_reverse_iterator = double_node_iterator<const Node, const quick_link, const typename quick_link::BackwardLink, const typename quick_link::ForwardLink,
            const T, &Node::m_val, vector_set>;

        using allocator_type = Allocator;
        using key_compare = Compare;
        using value_compare = Compare;

        vector_set() : m_tree(Compare{}), m_nodeAlloc(m_alloc) {}

        vector_set(Compare comp, const Allocator& alloc = Allocator()) : m_tree(comp), m_alloc(alloc), m_nodeAlloc(m_alloc) {}

        vector_set(const vector_set& other) : m_tree(other.m_tree.m_comp), m_alloc(other.m_alloc), m_nodeAlloc(other.m_nodeAlloc)
        {
            CopyElements(other);
        }

        vector_set(vector_set&& other) noexcept : m_tree(std::move(other.m_tree)), m_alloc(std::move(other.m_alloc)), m_nodeAlloc(std::move(other.m_nodeAlloc))
        {
            other.m_tree.m_root = nullptr;
        }

        vector_set(std::initializer_list<value_type> init, const Compare& comp = Compare(), const Allocator& alloc = Allocator()) : m_tree(comp), m_alloc(alloc)
        {
            //It is not clear how to move the elemetns from std::initializer_list,
            //see https://stackoverflow.com/questions/8193102/initializer-list-and-move-semantics
            //and https://stackoverflow.com/questions/36377758/how-to-move-elements-of-an-initializer-list/36411040#36411040
            for (const value_type & val : init)
            {
                insert(val);
            }
        }

        vector_set(std::initializer_list<value_type> init, const Allocator& alloc)
            : vector_set(init, Compare(), alloc)
        {
        }

        template <class InputIt>
        vector_set(InputIt first, InputIt last, const Compare& comp = Compare(), const Allocator& alloc = Allocator()) :
            vector_set(comp, alloc)
        {
            //We do not have insert() with two argumets.
            //std::copy(first, last, std::inserter(*this, begin()));
            std::for_each(first, last, [this](const T & val) { insert(val); });
        }

        template <class InputIt>
        vector_set(InputIt first, InputIt last, const Allocator& alloc = Allocator()) :
            vector_set(first, last, Compare(), alloc)
        {
        }

        ~vector_set()
        {
            clear();
        }

        vector_set & operator = (const vector_set & other)
        {
            clear();
            //Should we copy the allocator?
            m_tree.m_comp = other.m_tree.m_comp;
            CopyElements(other);
            return *this;
        }

        vector_set & operator = (vector_set && other) noexcept
        {
            clear();
            m_tree = std::move(other.m_tree);
            m_alloc = std::move(other.m_alloc);
            m_nodeAlloc = std::move(other.m_nodeAlloc);
            other.m_tree.m_root = nullptr;
            return *this;
        }

        bool operator == (const vector_set & other) const
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

        bool operator != (const vector_set & other) const
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

        std::pair<iterator, bool> insert(const value_type & m_val)
        {
            return UniversalInsert(m_val);
        }

        std::pair<iterator, bool> insert(value_type && m_val)
        {
            return UniversalInsert(std::move(m_val));
        }

        template <class... Args>
        std::pair<iterator, bool> emplace(Args&&... args)
        {
            Node * parent;
            NodeHolder val_node(CreateNode(std::forward<Args>(args) ...), NodeDeleter{this});
            Node * node = m_tree.FindNodeByKey(val_node->m_val, &parent);
            const bool exists = node != nullptr;

            if (!exists)
            {
                node = val_node.release();
                m_tree.InsertNode(node, parent);
            }

            return std::make_pair(iterator(typename List::iterator(node)), !exists);
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
        const_iterator find(const Key & key) const
        {
            return NodeToConstIterator(m_tree.FindNodeByKey(key));
        }

        template <class Key>
        iterator find(const Key & key)
        {
            return NodeToIterator(m_tree.FindNodeByKey(key));
        }

        //Calculating the index requires the iteration from the root
        //and the calculation the sum of number of the elements in the left subtrees
        //of the parent nodes.
        template <class Key>
        std::tuple<const_iterator, size_type> find2(const Key & key) const
        {
            auto [node, index] = FindIndexByKey(key);

            return std::make_tuple(NodeToConstIterator(node), index);
        }

        template <class Key>
        std::tuple<iterator, size_type> find2(const Key & key)
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
        bool contains(const Key & key) const
        {
            return m_tree.FindNodeByKey(key) != nullptr;
        }

        template <class Key>
        const_iterator lower_bound(const Key & key) const
        {
            return NodeToConstIterator(std::get<0>(m_tree.FindBoundByKey(key)));
        }

        template <class Key>
        iterator lower_bound(const Key & key)
        {
            return NodeToIterator(std::get<0>(m_tree.FindBoundByKey(key)));
        }

        template <class Key>
        const_iterator upper_bound(const Key & key) const
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
        iterator upper_bound(const Key & key)
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
        size_type index_of(const Key & key) const
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

            //Remove the node from the list.
            DestroyNode(z);
        }

        //Retutns the number of removed elements.
        template <class Key>
        size_type erase(const Key & key)
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
            while (!m_tree.m_list.empty())
            {
                //We destroy the node that is still included to the list.
                DestroyNode(m_tree.m_list.front());
            }

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

        allocator_type get_allocator() const
        {
            return m_alloc;
        }

    private:

        const_iterator NodeToConstIterator(const Node * node) const
        {
            if (node != nullptr)
            {
                return const_iterator(typename List::const_iterator(node));
            }

            return end();
        }

        iterator NodeToIterator(Node * node)
        {
            if (node != nullptr)
            {
                return iterator(typename List::iterator(node));
            }

            return end();
        }

        template <class V>
        std::pair<iterator, bool> UniversalInsert(V && val)
        {
            Node * parent;
            Node * node = m_tree.FindNodeByKey(val, &parent);
            const bool exists = node != nullptr;

            if (!exists)
            {
                node = CreateNode(std::forward<V>(val));
                m_tree.InsertNode(node, parent);
            }

            return std::make_pair(iterator(typename List::iterator(node)), !exists);
        }

        void CopyElements(const vector_set & other)
        {
            for (const T & val : other)
            {
                insert(val);
            }
        }

        void CheckPosition(size_type pos) const
        {
            if (!(pos < size()))
            {
                throw std::out_of_range(aformat() << "Index " << pos << " is out of range [0, " << size() << "].");
            }
        }

        helpers::RedBlackTree<Node, T, Compare> m_tree;

        Allocator m_alloc;
        NodeAllocator m_nodeAlloc;

        friend class VectorSetTest;
    };
}
