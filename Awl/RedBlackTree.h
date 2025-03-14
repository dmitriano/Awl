/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/QuickList.h"
#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

#include <memory>
#include <tuple>
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <algorithm>

namespace awl::helpers
{
    template <class Node>
    struct RedBlackLink : public quick_link
    {
        enum class Color : uint8_t { Red, Black };

        Node* parent;
        Node* left;
        Node* right;
        Color color;
        //The number of the children.
        std::size_t count;

        //The number of the elements in the left subtree.
        size_t GetLeftCount() const
        {
            return this->left != nullptr ? this->left->count + 1 : 0;
        }

        //The number of the elements in the right subtree.
        size_t GetRightCount() const
        {
            return this->right != nullptr ? this->right->count + 1 : 0;
        }

        void UpdateCount()
        {
            //Save the old value to check if it changed.
            const std::size_t old_count = this->count;

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

        void SetParent(Node* p)
        {
            this->parent = p;

            if (this->parent != nullptr)
            {
                this->parent->UpdateCount();
            }
        }

        void SetLeft(Node* l)
        {
            this->left = l;
            UpdateCount();
        }

        void SetRight(Node* r)
        {
            this->right = r;
            UpdateCount();
        }

        void ClearRelations()
        {
            *this = {};
        }
    };

    template <class Node, class T, class Compare = std::less<>> requires std::is_base_of_v<RedBlackLink<Node>, Node>
    struct RedBlackTree
    {
        using List = quick_list<Node>;

        using Color = typename RedBlackLink<Node>::Color;

        RedBlackTree(Compare comp) : m_comp(std::move(comp)) {}

        bool empty() const
        {
            return m_root == nullptr;
        }

        std::size_t size() const
        {
            return m_root == nullptr ? 0 : m_root->count + 1;
        }

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

        template <class Key>
        std::tuple<Node *, size_t> FindIndexByKey(const Key & key) const
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
                return std::make_tuple(found_node, index);
            }

            return std::make_tuple(nullptr, static_cast<size_t>(-1));
        }

        //Returns an iterator pointing to the first element that is not less than (i.e. greater or equal to) key.
        template <class Key>
        std::tuple<Node *, bool> FindBoundByKey(const Key & key) const
        {
            Node * x = m_root;
            //the last found element greater than x
            Node * greater = nullptr;

            //walk down the tree
            while (x != nullptr)
            {
                if (m_comp(key, x->value))
                {
                    greater = x;
                    x = x->left;
                }
                else if (m_comp(x->value, key))
                {
                    x = x->right;
                }
                else
                {
                    //we found the equal element
                    return std::make_tuple(x, true);
                }
            }

            return std::make_tuple(greater, false);
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

            return nullptr;
        }

        size_t IndexOfNode(const Node * node) const
        {
            size_t index = node->GetLeftCount();

            const Node * x = node;

            while (true)
            {
                const Node * parent = x->parent;

                if (parent == nullptr)
                {
                    break;
                }

                if (x == parent->right)
                {
                    index += parent->GetLeftCount() + 1;
                }

                x = parent;
            }

            return index;
        }

        //The only function that requires this to be Node *.
        void CopyFrom(Node* this_node, Node* other)
        {
            this_node->count = other->count;

            if (other->left != nullptr)
            {
                other->left->parent = this_node;
            }

            this_node->left = other->left;

            if (other->right != nullptr)
            {
                other->right->parent = this_node;
            }

            this_node->right = other->right;

            //Replace other with this_node in the parent node.
            if (other->parent != nullptr)
            {
                if (other->parent->left == other)
                {
                    other->parent->SetLeft(this_node);
                }
                else
                {
                    other->parent->SetRight(this_node);
                }
            }

            this_node->color = other->color;
            this_node->SetParent(other->parent);
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
                assert(node == m_root || m_comp(node->value, m_list.front()->value));
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

        /*
        Rotate our tree Left
        
                    X        rb_left_rotate(X)--->            Y
                  /   \                                     /   \
                 A     Y                                   X     C
                     /   \                               /   \
                    B     C                             A     B
        
        N.B. This does not change the ordering.
        
        We assume that neither X or Y is NULL
        */
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

        /*
        Rotate our tree Right
        
                    X                                         Y
                  /   \                                     /   \
                 A     Y     <---rb_right_rotate(Y)        X     C
                     /   \                               /   \
                    B     C                             A     B
        
        N.B. This does not change the ordering.
        
        We assume that neither X or Y is NULL
        */
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
                CopyFrom(y, z);

                if (z == m_root)
                    m_root = y;

                //we do this all above instead of the following line in original code
                //to provide guarantee of the persistence of the node in the tree
                //z.mKey = y.mKey;
            }

            if (y->color == Color::Black && x != nullptr)
                BalanceAfterRemove(x);
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

        Compare m_comp;
        Node * m_root = nullptr;
        List m_list;
    };
}
