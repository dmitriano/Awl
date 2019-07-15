#pragma once

#include <iterator>
#include <assert.h>

namespace awl
{
    template<
        class Key,
        class T,
        class Compare = std::less<Key>
    > 
    class hybrid_map
    {
    private:

        enum class Color { Red, Black };

        struct Link
        {
            Link * parent;
            Link * left;
            Link * right;
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
                    UpdateCount(this->parent);
                }
            }

            void SetParent(Link * p)
            {
                assert(p != nullptr);
                this->parent = p;
                UpdateCount(this->parent);
            }

            void SetLeft(Link * l)
            {
                this->left = l;
                UpdateCount();
            }

            void SetRight(Link * r)
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
            Key key;
            T value;
        };
    };
}
