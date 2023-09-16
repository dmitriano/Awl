/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdexcept>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <algorithm>
#include <map>

namespace awl
{
    template<
        class Key,
        class T,
        class Compare = std::less<Key>,
        class Allocator = std::allocator<std::pair<const Key, T>>
    >
    class interval_map
    {
    public:

        using key_type = T;
        using mapped_type = T;
        using value_type = std::pair<const Key, T>;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;

    public:

        //using iterator = ring_iterator<T>;
        //using const_iterator = ring_iterator<const T>;
        //using reverse_iterator = std::reverse_iterator<iterator>;
        //using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        const T& at(const Key& key) const
        {
            auto* right_value = find_interval(key);

            if (right_value != nullptr)
            {
                if (before_right(key, *right_value))
                {
                    return right_value->value;
                }
            }

            throw std::out_of_range("Specified key does not exist.");
        }

        bool contains(const Key& key) const
        {
            auto* right_value = find_interval(key);

            return right_value != nullptr && before_right(key, *right_value);
        }

        T& at(const Key& key)
        {
            // The object itself is non-const, and casting away the const is allowed.
            return const_cast<T&>(const_cast<const interval_map*>(this)->at(key));
        }

        template <class KeyA, class KeyB, class U>
        void assign(KeyA&& a, KeyB&& b, U&& value)
        {
            if (less(a, b))
            {
                typename RightMap::iterator remove_begin;
                typename RightMap::iterator remove_end;

                {
                    auto b_i = m_map.lower_bound(b);

                    if (b_i != m_map.end())
                    {
                        RightValue& b_right_value = b_i->second;

                        if (b_right_value.rightKey == b)
                        {
                            // remove the interval
                            remove_end = std::next(b_i);
                        }
                        else
                        {
                            // trim the interval by offsetting its left bound to the right
                            RightValue saved_right_value = std::move(b_right_value);

                            m_map.erase(b_i);

                            auto [new_i, inserted] = m_map.emplace(next_key(std::forward<Key>(b)), std::move(saved_right_value));

                            if (!inserted)
                            {
                                throw std::runtime_error("Internal error: duplicate interval 1.");
                            }

                            remove_end = new_i;
                        }
                    }
                    else
                    {
                        remove_end = m_map.end();
                    }
                }

                {
                    auto a_i = m_map.lower_bound(a);

                    if (a_i != m_map.end())
                    {
                        if (a_i->first == a)
                        {
                            // remove the interval
                            remove_begin = a_i;
                        }
                        else
                        {
                            // trim the interval by offsetting its right bound to the left
                            RightValue& a_right_value = a_i->second;

                            a_right_value.rightKey = previous_key(std::forward<Key>(a));

                            remove_begin = std::next(a_i);
                        }
                    }
                    else
                    {
                        remove_begin = m_map.end();
                    }
                }

                m_map.erase(remove_begin, remove_end);

                auto [new_i, inserted] = m_map.emplace(std::forward<Key>(a), RightValue{ std::forward<Key>(b), std::forward<T>(value)});

                if (!inserted)
                {
                    throw std::runtime_error("Internal error: duplicate interval 2.");
                }
            }
        }

    private:

        using key_compare = Compare;

        key_compare key_comp() const
        {
            return m_map.key_comp();
        }

        bool less(const Key& left, const Key& right) const
        {
            return key_comp()(left, right);
        }

        bool equal(const Key& left, const Key& right) const
        {
            return !less(left, right) && !less(right, left);
        }

        bool less_or_equal(const Key& left, const Key& right) const
        {
            const bool saved_less = less(left, right);
            
            return saved_less || (!saved_less && !less(right, left));
        }

        bool greater(const Key& left, const Key& right) const
        {
            return !less_or_equal(left, right);
        }

        struct RightValue
        {
            Key rightKey;
            T value;
        };

        // Returns the interval the key belongs.
        const RightValue* find_interval(const Key& key) const
        {
            auto i = m_map.lower_bound(key);

            if (i != m_map.end())
            {
                return &i->second;
            }

            return nullptr;
        }

        RightValue* find_interval(const Key& key)
        {
            // The object itself is non-const, and casting away the const is allowed.
            return const_cast<RightValue&>(const_cast<const interval_map*>(this)->find_interval(key));
        }

        bool before_right(const Key& key, const RightValue& right_value) const
        {
            const Key& right_key = right_value.rightKey;

            // right_key can be max int.
            return less_or_equal(key, right_key);
        }

        template <class KeyU>
        Key next_key(KeyU&& key)
        {
            // implemented assuming Key is an integral type
            static_assert(std::is_integral_v<Key>);
            Key next = std::forward<Key>(key) + 1;

            if (next < key)
            {
                throw std::runtime_error("+1 overflow");
            }

            return next;
        }

        template <class KeyU>
        Key previous_key(KeyU&& key)
        {
            // implemented assuming Key is an integral type
            static_assert(std::is_integral_v<Key>);
            Key prev = std::forward<Key>(key) - 1;

            if (prev > key)
            {
                throw std::runtime_error("-1 overflow");
            }

            return prev;
        }

        using RightAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<const Key, RightValue>>;

        using RightMap = std::map<Key, RightValue, Compare, RightAllocator>;

        RightMap m_map;
    };
}
