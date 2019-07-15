#pragma once

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
                assert(p != nullptr);
                this->parent = p;
                this->parent->UpdateCount();
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

            void CopyFrom(Link * other)
            {
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
                this->count = other->count;
                this->rank = other->rank;
            }

            void ClearRelations()
            {
                *this = {};
            }
        };

        struct Node : public Link
        {
            T value;
        };

    public:

        hybrid_set(Compare comp = {}) : m_comp(comp)
        {
        }

    private:

        //p_parent is a node to which a new node is added.
        template <class Key>
        Node * FindNodeByKey(const Key & key, Node ** p_parent = nullptr)
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

        bool InsertNode(Node * node)
        {
            Node * parent;
            Node * x = FindNodeByKey(node->value, &parent);

            if (x != nullptr)
            {
                //A node with the same key already exists.
                return false;
            }

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

            return true;
        }

        void Balance(Node * node)
        {
            static_cast<void>(node);
        }

        Node * m_root = nullptr;
        std::size_t m_count = 0u;
        Compare m_comp;

        friend class HybridSetTest;
    };
}
