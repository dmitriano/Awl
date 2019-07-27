#pragma once

#include "QuickList.h"
#include "TransformIterator.h"
#include "Exception.h"
#include "StringFormat.h"

#include <iterator>
#include <memory>
#include <initializer_list>
#include <assert.h>

namespace awl
{
    template<class T, class Compare = std::less<>, class Allocator = std::allocator<T>> 
    class hybrid_set
    {
    private:

        enum class Color { Red, Black };

        struct Node;
        
        struct Link
        {
            Node * parent;
            Node * left;
            Node * right;
            Color color;
            //The number of the children.
            std::size_t count;

            //The number of the elements in the left subtree.
            size_t GetLeftCount() const
            {
                return this->left != nullptr ? this->left->count + 1: 0;
            }

            //The number of the elements in the right subtree.
            size_t GetRightCount() const
            {
                return this->right != nullptr ? this->right->count + 1 : 0;
            }

            void UpdateCount()
            {
                //Save the old value to check if it changed.
                std::size_t old_count = this->count;

                this->count = 0;

                if (this->left != nullptr)
                {
                    this->count = this->left->count + 1;
                }

                if (this->right != nullptr)
                {
                    this->count += this->right->count + 1;
                }

                if (this->count != old_count && this->parent != nullptr)
                {
                    this->parent->UpdateCount();
                }
            }

            void SetParent(Node * p)
            {
                this->parent = p;

                if (this->parent != nullptr)
                {
                    this->parent->UpdateCount();
                }
            }

            void SetLeft(Node * l)
            {
                this->left = l;
                UpdateCount();
            }

            void SetRight(Node * r)
            {
                this->right = r;
                UpdateCount();
            }

            void ClearRelations()
            {
                *this = {};
            }
        };

        struct Node : public Link, public quick_link
        {
            Node(const T & v) : Link{}, value(v)
            {
            }

            Node(T && v) : Link{}, value(std::move(v))
            {
            }

            template <class... Args>
            Node(Args&&... args) : Link{}, value(std::forward<Args>(args) ...)
            {
            }

            ~Node()
            {
                //emplace(Args...) method may construct the node even if there already is a node with the key in the container,
                //in which case the newly constructed element will be destroyed immediately. So a node is not guaranteed
                //to be included.
                safe_exclude();
            }
            
            //The only function that requires this to be Node *.
            void CopyFrom(Node * other)
            {
                this->count = other->count;

                if (other->left != nullptr)
                {
                    other->left->parent = this;
                }

                this->left = other->left;

                if (other->right != nullptr)
                {
                    other->right->parent = this;
                }

                this->right = other->right;

                //Replace other with this in the parent node.
                if (other->parent != nullptr)
                {
                    if (other->parent->left == other)
                    {
                        other->parent->SetLeft(this);
                    }
                    else
                    {
                        other->parent->SetRight(this);
                    }
                }

                this->color = other->color;
                this->SetParent(other->parent);
            }

            T value;
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

        void DestroyNode(Node * node)
        {
            node->~Node();
            m_nodeAlloc.deallocate(node, 1);
        }

        using List = quick_list<Node>;

        struct IteratorHelper
        {
            static auto MakeXFunc()
            {
                return [](Node * node) -> T & { return node->value; };
            }

            static auto MakeConstXFunc()
            {
                return [](const Node * node) -> const T & { return node->value; };
            }
        };

        using XFunc = decltype(IteratorHelper::MakeXFunc());
        using ConstXFunc = decltype(IteratorHelper::MakeConstXFunc());

        template <class ListIterator>
        static transform_iterator<XFunc, ListIterator, hybrid_set> MakeIterator(ListIterator i)
        {
            return make_friend_iterator<hybrid_set>(i, IteratorHelper::MakeXFunc());
        }

        template <class ListIterator>
        static transform_iterator<ConstXFunc, ListIterator, hybrid_set> MakeConstIterator(ListIterator i)
        {
            return make_friend_iterator<hybrid_set>(i, IteratorHelper::MakeConstXFunc());
        }

    public:

        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type & ;
        using const_reference = const value_type & ;

        using iterator = transform_iterator<XFunc, typename List::iterator, hybrid_set>;
        using const_iterator = transform_iterator<ConstXFunc, typename List::const_iterator, hybrid_set>;

        using reverse_iterator = transform_iterator<XFunc, typename List::reverse_iterator, hybrid_set>;
        using const_reverse_iterator = transform_iterator<ConstXFunc, typename List::const_reverse_iterator, hybrid_set>;

        using allocator_type = Allocator;
        using key_compare = Compare;
        using value_compare = Compare;

        hybrid_set() : m_nodeAlloc(m_alloc)
        {
        }
        
        hybrid_set(Compare comp, const Allocator& alloc = Allocator()) : m_comp(comp), m_alloc(alloc), m_nodeAlloc(m_alloc)
        {
        }

        hybrid_set(const hybrid_set & other) : m_comp(other.m_comp), m_alloc(other.m_alloc), m_nodeAlloc(other.m_nodeAlloc)
        {
            CopyElements(other);
        }

        hybrid_set(hybrid_set && other) : m_comp(std::move(other.m_comp)), m_alloc(std::move(other.m_alloc)), m_nodeAlloc(std::move(other.m_nodeAlloc)),
            m_root(std::move(other.m_root)), m_list(std::move(other.m_list))
        {
            other.m_root = nullptr;
        }

        hybrid_set(std::initializer_list<value_type> init, const Compare& comp = Compare(), const Allocator& alloc = Allocator()) : m_comp(comp), m_alloc(alloc)
        {
            //It is not clear how to move the elemetns from std::initializer_list,
            //see https://stackoverflow.com/questions/8193102/initializer-list-and-move-semantics
            //and https://stackoverflow.com/questions/36377758/how-to-move-elements-of-an-initializer-list/36411040#36411040
            for (const value_type & val : init)
            {
                insert(val);
            }
        }

        hybrid_set(std::initializer_list<value_type> init, const Allocator& alloc)
            : hybrid_set(init, Compare(), alloc)
        {
        }

        ~hybrid_set()
        {
            clear();
        }

        hybrid_set & operator = (const hybrid_set & other)
        {
            clear();
            //Should we copy the allocator?
            m_comp = other.m_comp;
            CopyElements(other);
        }

        hybrid_set & operator = (hybrid_set && other)
        {
            clear();
            m_comp = std::move(other.m_comp);
            m_alloc = std::move(other.m_alloc);
            m_nodeAlloc = std::move(other.m_nodeAlloc);
            m_root = std::move(other.m_root);
            m_list = std::move(other.m_list);
            other.m_root = nullptr;
        }

        bool operator == (const hybrid_set & other) const
        {
            if (size() == other.size())
            {
                const_iterator i = begin();
                for (const value_type & val : other)
                {
                    if (m_comp(val, *i) || m_comp(*i, val))
                    {
                        return false;
                    }

                    ++i;
                }

                return true;
            }

            return false;
        }

        bool operator != (const hybrid_set & other) const
        {
            return !operator == (other);
        }

        T & front() { return m_list.front()->value; }
        const T & front() const { return m_list.front()->value; }

        T & back() { return m_list.back()->value; }
        const T & back() const { return m_list.back()->value; }

        iterator begin() { return MakeIterator(m_list.begin()); }
        const_iterator begin() const { return MakeConstIterator(m_list.begin()); }

        iterator end() { return MakeIterator(m_list.end()); }
        const_iterator end() const { return MakeConstIterator(m_list.end()); }

        reverse_iterator rbegin() { return MakeIterator(m_list.rbegin()); }
        const_reverse_iterator rbegin() const { return MakeConstIterator(m_list.rbegin()); }

        reverse_iterator rend() { return MakeIterator(m_list.rend()); }
        const_reverse_iterator rend() const { return MakeConstIterator(m_list.rend()); }

        std::pair<iterator, bool> insert(const value_type & value)
        {
            return UniversalInsert(value);
        }

        std::pair<iterator, bool> insert(value_type && value)
        {
            return UniversalInsert(std::move(value));
        }

        template <class... Args>
        std::pair<iterator, bool> emplace(Args&&... args)
        {
            Node * parent;
            std::unique_ptr<Node> val_node(CreateNode(std::forward<Args>(args) ...));
            Node * node = FindNodeByKey(val_node->value, &parent);
            const bool exists = node != nullptr;

            if (!exists)
            {
                node = val_node.release();
                InsertNode(node, parent);
            }

            return std::make_pair(MakeIterator(typename List::iterator(node)), !exists);
        }

        bool empty() const
        {
            return m_root == nullptr;
        }

        size_type size() const
        {
            return m_root == nullptr ? 0 : m_root->count + 1;
        }

        template <class Key>
        iterator find(const Key & key)
        {
            Node * node = FindNodeByKey(key);

            if (node != nullptr)
            {
                return MakeIterator(typename List::iterator(node));
            }

            return end();
        }

        reference at(size_type pos)
        {
            return FindNodeByIndex(pos)->value;
        }

        const_reference at(size_type pos) const
        {
            return FindNodeByIndex(pos)->value;
        }

        template <class Key>
        size_type index_of(const Key & key) const
        {
            size_t parent_rank = 0;
            const size_t size = this->size();

            Node * found_node = FindNodeByKey(key,
                [](Node *)
                {
                },
                [&parent_rank, size](Node * node)
                {
                    //The parent and its children are at the left side.
                    parent_rank += node->GetLeftCount() + 1;
                    assert(parent_rank <= size);
                }
            );

            if (found_node != nullptr)
            {
                const size_t index = found_node->GetLeftCount() + parent_rank;
                assert(index < size);
                return index;
            }

            static_cast<void>(size);

            throw GeneralException(_T("Key not found."));
        }

        template <class Key>
        const_iterator find(const Key & key) const
        {
            Node * node = FindNodeByKey(key);

            if (node != nullptr)
            {
                return MakeConstIterator(typename List::const_iterator(node));
            }

            return end();
        }

        void erase(iterator i)
        {
            List::iterator li = ExtractListIterator(i);

            Node * node = *li;

            RemoveNode(node);
        }

        template <class Key>
        bool erase(const Key & key)
        {
            iterator i = find(key);

            if (i != end())
            {
                erase(i);
                return true;
            }

            return false;
        }

        void clear()
        {
            while (!m_list.empty())
            {
                //We destroy the node that is still included to the list.
                DestroyNode(m_list.front());
            }

            m_root = nullptr;
        }

    private:

        template <class Key>
        Node * FindNodeByKey(const Key & key) const
        {
            auto accum = [](Node *) {};
            
            return FindNodeByKey(key, accum, accum);
        }

        //p_parent is a node to which a new node is added.
        template <class Key>
        Node * FindNodeByKey(const Key & key, Node ** p_parent) const
        {
            if (p_parent != nullptr)
            {
                *p_parent = nullptr;

                auto accum = [&p_parent](Node * x)
                {
                    *p_parent = x;
                };

                return FindNodeByKey(key, accum, accum);
            }

            return FindNodeByKey(key);
        }

        template <class Key, class AccumulateLeft, class AccumulateRight>
        Node * FindNodeByKey(const Key & key, AccumulateLeft && accum_left, AccumulateRight && accum_right) const
        {
            Node * x = m_root;

            //walk down the tree
            while (x != nullptr)
            {
                if (m_comp(key, x->value))
                {
                    accum_left(x);
                    x = x->left;
                }
                else if (m_comp(x->value, key))
                {
                    accum_right(x);
                    x = x->right;
                }
                else
                {
                    return x;
                }
            }

            return nullptr;
        }

        Node * FindNodeByIndex(size_t index) const
        {
            Node * x = m_root;
            size_t i = index;

            //walk down the tree
            while (x != nullptr)
            {
                const size_t rank = x->GetLeftCount();

                if (i < rank)
                {
                    x = x->left;
                }
                else if (i > rank)
                {
                    i -= (rank + 1);
                    x = x->right;
                }
                else
                {
                    return x;
                }
            }

            throw std::out_of_range(basic_format<char>() << "Index " << index << " is out of range [0, " << size() << "].");
        }

        template <class V>
        std::pair<iterator, bool> UniversalInsert(V && val)
        {
            Node * parent;
            Node * node = FindNodeByKey(val, &parent);
            const bool exists = node != nullptr;

            if (!exists)
            {
                node = CreateNode(std::forward<V>(val));
                InsertNode(node, parent);
            }

            return std::make_pair(MakeIterator(typename List::iterator(node)), !exists);
        }

        //Inserts a node that does not exist to the specified parent.
        void InsertNode(Node * node, Node * parent)
        {
            node->parent = parent;

            if (parent == nullptr)
            {
                m_root = node;
            }
            else
            {
                if (m_comp(node->value, parent->value))
                {
                    parent->SetLeft(node);
                }
                else
                {
                    //They cannot be equal, because we passed the parent in FindNodeByKey(...).
                    assert(m_comp(parent->value, node->value));
                    parent->SetRight(node);
                }
            }
            node->color = Color::Red;
            BalanceAfterInsert(node);
            m_root->color = Color::Black;

            //Insert newly added node to the list.
            Node * pred = GetPredecessor(node);
            if (pred != nullptr)
            {
                assert(m_comp(pred->value, node->value));
                m_list.insert(typename List::iterator(pred), node);
            }
            else
            {
                assert(node == m_root || m_comp(node->value, front()));
                m_list.push_front(node);
            }
        }

        //Returns the pointer to the smallest node greater than x.
        Node * GetSuccessor(Node * x)
        {
            Node * y;

            if (x->right != nullptr)
            {
                // If right is not NULL then go right one and
                // then keep going left until we find a node with
                // no left pointer.
                for (y = x->right; y->left != nullptr; y = y->left);
            }
            else
            {
                // Go up the tree until we get to a node that is on the
                // left of its parent (or the root) and then return the
                // parent.
                y = x->parent;
                while (y != nullptr && x == y->right)
                {
                    x = y;
                    y = y->parent;
                }
            }
            return y;
        }

        //Returns the pointer to the largest node smaller than x.
        Node * GetPredecessor(Node * x)
        {
            Node * y;

            if (x->left != nullptr)
            {
                // If left is not NULL then go left one and
                // then keep going right until we find a node with
                // no right pointer.
                for (y = x->left; y->right != nullptr; y = y->right);
            }
            else
            {
                // Go up the tree until we get to a node that is on the
                // right of its parent (or the root) and then return the
                // parent.
                y = x->parent;
                while (y != nullptr && x == y->left)
                {
                    x = y;
                    y = y->parent;
                }
            }
            return y;
        }

        // Rotate our tree Left
        //
        //             X        rb_left_rotate(X)--->            Y
        //           /   \                                     /   \
        //          A     Y                                   X     C
        //              /   \                               /   \
        //             B     C                             A     B
        //
        // N.B. This does not change the ordering.
        //
        // We assume that neither X or Y is NULL
        void RotateLeft(Node * x)
        {
            Node * y = x->right;

            // Turn Y's left subtree into X's right subtree (move B)
            x->right = y->left;

            // If B is not nullptr, set it's parent to be X
            if (y->left != nullptr)
                y->left->parent = x;

            // Set Y's parent to be what X's parent was
            y->parent = x->parent;

            // if X was the root
            if (x->parent == nullptr)
                m_root = y;
            else
            {
                // Set X's parent's left or right pointer to be Y
                if (x == x->parent->left)
                    x->parent->left = y;
                else
                    x->parent->right = y;
            }

            // Put X on Y's left
            y->left = x;

            // Set X's parent to be Y
            x->parent = y;

            x->UpdateCount();
        }

        // Rotate our tree Right
        //
        //             X                                         Y
        //           /   \                                     /   \
        //          A     Y     <---rb_right_rotate(Y)        X     C
        //              /   \                               /   \
        //             B     C                             A     B
        //
        // N.B. This does not change the ordering.
        //
        // We assume that neither X or Y is NULL
        void RotateRight(Node * y)
        {
            Node * x = y->left;

            // Turn X's right subtree into Y's left subtree (move B)
            y->left = x->right;

            // If B is not nullptr, set it's parent to be Y
            if (x->right != nullptr)
                x->right->parent = y;

            // Set X's parent to be what Y's parent was
            x->parent = y->parent;

            // if Y was the root
            if (y->parent == nullptr)
                m_root = x;
            else
            {
                // Set Y's parent's left or right pointer to be X
                if (y == y->parent->left)
                    y->parent->left = x;
                else
                    y->parent->right = x;
            }

            // Put Y on X's right
            x->right = y;

            // Set Y's parent to be X
            y->parent = x;

            y->UpdateCount();
        }

        //Balance tree past inserting
        void BalanceAfterInsert(Node * z)
        {
            //Having added a red node, we must now walk back up the tree balancing
            //it, by a series of rotations and changing of colours
            Node * x = z;
            Node * y;

            //While we are not at the top and our parent node is red
            //N.B. Since the root node is garanteed black, then we
            //are also going to stop if we are the child of the root
            while (x != m_root && (x->parent->color == Color::Red))
            {
                //if our parent is on the left side of our grandparent
                if (x->parent == x->parent->parent->left)
                {
                    //get the right side of our grandparent (uncle?)
                    y = x->parent->parent->right;
                    if (y != nullptr && y->color == Color::Red)
                    {
                        //make our parent black
                        x->parent->color = Color::Black;
                        //make our uncle black
                        y->color = Color::Black;
                        //make our grandparent red
                        x->parent->parent->color = Color::Red;
                        //now consider our grandparent
                        x = x->parent->parent;
                    }
                    else
                    {
                        //if we are on the right side of our parent
                        if (x == x->parent->right)
                        {
                            //Move up to our parent
                            x = x->parent;
                            RotateLeft(x);
                        }

                        /* make our parent black */
                        x->parent->color = Color::Black;
                        /* make our grandparent red */
                        x->parent->parent->color = Color::Red;
                        /* right rotate our grandparent */
                        RotateRight(x->parent->parent);
                    }
                }
                else
                {
                    //everything here is the same as above, but
                    //exchanging left for right
                    y = x->parent->parent->left;
                    if (y != nullptr && y->color == Color::Red)
                    {
                        x->parent->color = Color::Black;
                        y->color = Color::Black;
                        x->parent->parent->color = Color::Red;

                        x = x->parent->parent;
                    }
                    else
                    {
                        if (x == x->parent->left)
                        {
                            x = x->parent;
                            RotateRight(x);
                        }

                        x->parent->color = Color::Black;
                        x->parent->parent->color = Color::Red;
                        RotateLeft(x->parent->parent);
                    }
                }
            }
            m_root->color = Color::Black;
        }

        // Delete the node z, and free up the space
        void RemoveNode(Node * z)
        {
            Node * x;
            Node * y;

            if (z->left == nullptr || z->right == nullptr)
                y = z;
            else
                y = GetSuccessor(z);

            if (y->left != nullptr)
                x = y->left;
            else
                x = y->right;

            if (x != nullptr)
                x->SetParent(y->parent);

            if (y->parent == nullptr)
                m_root = x;
            else
            {
                if (y == y->parent->left)
                    y->parent->SetLeft(x);
                else
                    y->parent->SetRight(x);
            }

            if (y != z)
            {
                //we must replace 'z' with 'y' node
                y->CopyFrom(z);

                if (z == m_root)
                    m_root = y;

                //we do this all above instead of the following line in original code
                //to provide guarantee of the persistence of the node in the tree
                //z.mKey = y.mKey;
            }

            if (y->color == Color::Black && x != nullptr)
                BalanceAfterRemove(x);

            //Remove the node from the list.
            DestroyNode(z);
        }

        // Restores the reb-black properties after a delete.
        void BalanceAfterRemove(Node * x)
        {
            Node * w;

            while (x != m_root && x->color == Color::Black)
            {
                if (x == x->parent->left)
                {
                    w = x->parent->right;
                    if (w == nullptr)
                    {
                        x = x->parent;
                        continue;
                    }

                    if (w->color == Color::Red)
                    {
                        w->color = Color::Black;
                        x->parent->color = Color::Red;
                        RotateLeft(x->parent);
                        w = x->parent->right;
                    }

                    if (w == nullptr)
                    {
                        x = x->parent;
                        continue;
                    }

                    if ((w->left == nullptr || w->left->color == Color::Black) &&
                        (w->right == nullptr || w->right->color == Color::Black))
                    {
                        w->color = Color::Red;
                        x = x->parent;
                    }
                    else
                    {
                        if (w->right == nullptr || w->right->color == Color::Black)
                        {
                            if (w->left != nullptr)
                                w->left->color = Color::Black;
                            w->color = Color::Red;
                            RotateRight(w);
                            w = x->parent->right;
                        }

                        w->color = x->parent->color;
                        x->parent->color = Color::Black;
                        if (w->right != nullptr)
                            w->right->color = Color::Black;
                        RotateLeft(x->parent);
                        x = m_root;
                    }
                }
                else
                {
                    w = x->parent->left;
                    if (w == nullptr)
                    {
                        x = x->parent;
                        continue;
                    }

                    if (w->color == Color::Red)
                    {
                        w->color = Color::Black;
                        x->parent->color = Color::Red;
                        RotateRight(x->parent);
                        w = x->parent->left;
                    }

                    if (w == nullptr)
                    {
                        x = x->parent;
                        continue;
                    }

                    if ((w->right == nullptr || w->right->color == Color::Black) &&
                        (w->left == nullptr || w->left->color == Color::Black))
                    {
                        w->color = Color::Red;
                        x = x->parent;
                    }
                    else
                    {
                        if (w->left == nullptr || w->left->color == Color::Black)
                        {
                            if (w->right != nullptr)
                                w->right->color = Color::Black;
                            w->color = Color::Red;
                            RotateLeft(w);
                            w = x->parent->left;
                        }

                        w->color = x->parent->color;
                        x->parent->color = Color::Black;
                        if (w->left != nullptr)
                            w->left->color = Color::Black;
                        RotateRight(x->parent);
                        x = m_root;
                    }
                }
            }
            x->color = Color::Black;
        }

        static typename List::iterator ExtractListIterator(iterator i)
        {
            return i.m_i;
        }

        void CopyElements(const hybrid_set & other)
        {
            for (const T & val : other)
            {
                insert(val);
            }
        }
        
        Compare m_comp;
        Allocator m_alloc;
        NodeAllocator m_nodeAlloc;
        Node * m_root = nullptr;
        List m_list;

        friend class HybridSetTest;
    };
}
