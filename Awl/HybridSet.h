#pragma once

#include "QuickList.h"
#include "TransformIterator.h"

#include <iterator>
#include <assert.h>

namespace awl
{
    template<
        class T,
        class Compare = std::less<>
    > 
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
            std::size_t rank;
            std::size_t count;

            void UpdateCount()
            {
                //Save the old value to check if it changed.
                std::size_t old_count = this->count;

                this->count = 0;

                if (this->left != nullptr)
                {
                    this->count = this->left->count + 1;
                    this->rank = this->count + 1;
                }
                else
                {
                    this->rank = 1;
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
            
            //The only function that requires this to be Node *.
            void CopyFrom(Node * other)
            {
                this->count = other->count;
                this->rank = other->rank;

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

        hybrid_set(Compare comp = {}) : m_comp(comp)
        {
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

        bool empty() const { return m_list.empty(); }
        bool empty_or_contains_one() const { return m_list.empty_or_contains_one(); }
        bool contains_one() const { return m_list.contains_one(); }

        std::pair<iterator, bool> insert(const value_type & value)
        {
            std::pair<Node *, bool> p = InsertNode(value);
            //It cannot be null after insertion.
            assert(p.first != nullptr);
            return std::make_pair(MakeIterator(typename List::iterator(p.first)), p.second);
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
            List::iterator li = i.m_i;

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

    private:

        //p_parent is a node to which a new node is added.
        template <class Key>
        Node * FindNodeByKey(const Key & key, Node ** p_parent = nullptr) const
        {
            if (p_parent != nullptr)
            {
                *p_parent = nullptr;
            }

            Node * x = m_root;

            //walk down the tree
            while (x != nullptr)
            {
                if (p_parent != nullptr)
                {
                    *p_parent = x;
                }
                
                if (m_comp(key, x->value))
                {
                    x = x->left;
                }
                else if (m_comp(x->value, key))
                {
                    x = x->right;
                }
                else
                {
                    return x;
                }
            }

            return nullptr;
        }

        std::pair<Node *, bool> InsertNode(const T & val)
        {
            Node * parent;
            Node * x = FindNodeByKey(val, &parent);

            if (x != nullptr)
            {
                //A node with the same key already exists.
                return std::make_pair(x, false);
            }

            Node * node = new Node(val);
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
            Balance(node);
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

            return std::make_pair(node, true);
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
        void Balance(Node * z)
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
                FixAfterDelete(x);

            //Remove the node from the list.
            z->exclude();
            delete z;
        }

        // Restores the reb-black properties after a delete.
        void FixAfterDelete(Node * x)
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

        Node * m_root = nullptr;
        List m_list;
        Compare m_comp;

        friend class HybridSetTest;
    };
}
