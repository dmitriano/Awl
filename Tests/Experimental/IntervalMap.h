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

    private:

        // The right keys are in the map and left keys are in the values.
        struct RightValue
        {
            Key leftKey;
            T value;
        };

        using RightAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<const Key, RightValue>>;

        using RightMap = std::map<Key, RightValue, Compare, RightAllocator>;

        template <class Container, class Iterator, class Value>
        class MyIterator
        {
        public:

            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = std::pair<const Key, Value&>;
            using difference_type = std::ptrdiff_t;
            using reference = value_type&;
            using pointer = value_type*;

            MyIterator() = default;

            MyIterator(const MyIterator& other) = default;
            MyIterator(MyIterator&& other) = default;

            MyIterator& operator = (const MyIterator& other) = default;
            MyIterator& operator = (MyIterator&& other) = default;

            value_type operator* () const
            {
                return value_type
                {
                    m_i->second.leftKey + m_offset,
                    m_i->second.value
                };
            }

            MyIterator& operator++ ()
            {
                move_next();

                return *this;
            }

            MyIterator operator++ (int)
            {
                MyIterator tmp = *this;

                move_next();

                return tmp;
            }

            MyIterator& operator-- ()
            {
                move_prev();

                return *this;
            }

            MyIterator operator-- (int)
            {
                MyIterator tmp = *this;

                move_prev();

                return tmp;
            }

            bool operator == (const MyIterator& other) const
            {
                assert(m_pMap == other.m_pMap);
                
                return as_tie() == other.as_tie();
            }

            bool operator != (const MyIterator& other)  const
            {
                return !(*this == other);
            }

            bool operator < (const MyIterator& other) const
            {
                return as_tie() < other.as_tie();
            }

            // Conversion to const_iterator
            operator MyIterator<const Container, typename RightMap::const_iterator, const Value>() const
            {
                return MyIterator<const Container, typename RightMap::const_iterator, const Value>(m_pMap, m_i, m_offset);
            }

        private:

            MyIterator(Container* p_map, Iterator i, Key offset = 0) :
                m_pMap(p_map), m_i(i), m_offset(offset)
            {
            }

            void move_next()
            {
                assert(m_i != container().m_map.end());
                
                Key length = m_i->first - m_i->second.leftKey;
                assert(container().less_or_equal(m_offset, length));

                if (container().equal(m_offset, length))
                {
                    m_i = std::next(m_i);
                    m_offset = 0;
                }
                else
                {
                    m_offset = next_key(m_offset);
                }
            }

            void move_prev()
            {
                throw std::runtime_error("Not implemented.");
            }

            const interval_map& container() const
            {
                return *m_pMap;
            }

            auto as_tie() const
            {
                return std::tie(m_i, m_offset);
            }

            Container* m_pMap;

            Iterator m_i;

            Key m_offset;

            friend interval_map;
        };

    public:

        using iterator = MyIterator<interval_map, typename RightMap::iterator, T>;
        using const_iterator = MyIterator<const interval_map, typename RightMap::const_iterator, const T>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        iterator begin() { return iterator(this, m_map.begin()); }
        const_iterator begin() const { return cbegin(); }
        const_iterator cbegin() const { return iterator(this, m_map.begin()); }

        iterator end() { return iterator(this, m_map.end()); }
        const_iterator end() const { return cend(); }
        const_iterator cend() const { return iterator(this, m_map.end());}

        reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }
        const_reverse_iterator rbegin() const { return crbegin(); }
        const_reverse_iterator crbegin() const { return std::make_reverse_iterator(cend()); }

        reverse_iterator rend() { return std::make_reverse_iterator(begin()); }
        const_reverse_iterator rend() const { return crend(); }
        const_reverse_iterator crend() const { return std::make_reverse_iterator(cbegin()); }

        const T& at(const Key& key) const
        {
            auto* right_value = find_interval(key);

            if (right_value != nullptr)
            {
                if (after_left(key, *right_value))
                {
                    return right_value->value;
                }
            }

            throw std::out_of_range("Specified key does not exist.");
        }

        bool contains(const Key& key) const
        {
            auto* right_value = find_interval(key);

            return right_value != nullptr && after_left(key, *right_value);
        }

        T& at(const Key& key)
        {
            // The object itself is non-const, and casting away the const is allowed.
            return const_cast<T&>(const_cast<const interval_map*>(this)->at(key));
        }

        template <class KeyA, class KeyB, class U>
        void assign(KeyA&& a, KeyB&& b, U&& value)
        {
            if (less_or_equal(a, b))
            {
                typename RightMap::iterator remove_begin;
                typename RightMap::iterator remove_end;

                {
                    auto a_i = m_map.lower_bound(a);

                    if (a_i != m_map.end())
                    {
                        RightValue& a_right_value = a_i->second;

                        if (less(a_right_value.leftKey, a))
                        {
                            // trim the interval by offsetting its right bound to the left
                            RightValue saved_right_value = std::move(a_right_value);

                            Key saved_right_key = a_i->first;

                            m_map.erase(a_i);

                            Key new_right_key = previous_key(std::forward<KeyA>(a));
                            
                            auto [new_i, inserted] = m_map.emplace(new_right_key, std::move(saved_right_value));

                            if (!inserted)
                            {
                                throw std::runtime_error("Internal error: duplicate interval 1.");
                            }

                            if (less(b, saved_right_key))
                            {
                                // [a, b] inside the current interval, insert the tail with the same value.
                                auto [tail_i, tail_inserted] = m_map.emplace(saved_right_key, RightValue{ next_key(b), new_i->second.value });

                                if (!tail_inserted)
                                {
                                    throw std::runtime_error("Internal error: duplicate interval 3.");
                                }

                                // b is already handled.
                                remove_begin = m_map.end();
                            }
                            else
                            {
                                remove_begin = std::next(new_i);
                            }
                        }
                        else
                        {
                            // remove the interval
                            remove_begin = a_i;
                        }
                    }
                    else
                    {
                        remove_begin = m_map.end();
                    }
                }

                if (remove_begin != m_map.end())
                {
                    auto b_i = m_map.lower_bound(b);

                    if (b_i != m_map.end())
                    {
                        RightValue& b_right_value = b_i->second;

                        if (less_or_equal(b_right_value.leftKey, b))
                        {
                            // trim the interval by offsetting its left bound to the right
                            Key new_left_key = next_key(std::forward<KeyB>(b));

                            if (less_or_equal(new_left_key, b_i->first))
                            {
                                b_right_value.leftKey = new_left_key;

                                remove_end = b_i;
                            }
                            else
                            {
                                // interval is empty so remove it
                                remove_end = std::next(b_i);
                            }
                        }
                        else
                        {
                            // remove all before this interval
                            remove_end = b_i;
                        }
                    }
                    else
                    {
                        remove_end = m_map.end();
                    }

                    m_map.erase(remove_begin, remove_end);
                }

                auto [new_i, inserted] = m_map.emplace(std::forward<KeyA>(b), RightValue{ std::forward<KeyB>(a), std::forward<U>(value)});

                if (!inserted)
                {
                    throw std::runtime_error("Internal error: duplicate interval 3.");
                }

                //Merge adjacent intervals with the same value.

                {
                    if (new_i != m_map.begin())
                    {
                        auto prev_i = std::prev(new_i);

                        if (new_i->second.value == prev_i->second.value)
                        {
                            new_i->second.leftKey = prev_i->second.leftKey;

                            m_map.erase(prev_i);
                        }
                    }

                    {
                        auto next_i = std::next(new_i);

                        if (next_i != m_map.end() && new_i->second.value == next_i->second.value)
                        {
                            next_i->second.leftKey = new_i->second.leftKey;

                            m_map.erase(new_i);
                        }
                    }
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

        bool greater(const Key& left, const Key& right) const
        {
            return less(right, left);
        }

        bool equal(const Key& left, const Key& right) const
        {
            return !less(left, right) && !greater(left, right);
        }

        bool less_or_equal(const Key& left, const Key& right) const
        {
            return !greater(left, right);
        }

        bool greater_or_equal(const Key& left, const Key& right) const
        {
            return !less(left, right);
        }

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

        bool after_left(const Key& key, const RightValue& right_value) const
        {
            const Key& left_key = right_value.leftKey;

            // right_key can be max int.
            return greater_or_equal(key, left_key);
        }

        template <class KeyU>
        static Key next_key(KeyU&& key)
        {
            // implemented assuming Key is an integral type
            static_assert(std::is_integral_v<Key>);
            Key next = std::forward<KeyU>(key) + 1;

            if (next < key)
            {
                throw std::runtime_error("+1 overflow");
            }

            return next;
        }

        template <class KeyU>
        static Key previous_key(KeyU&& key)
        {
            // implemented assuming Key is an integral type
            static_assert(std::is_integral_v<Key>);
            Key prev = std::forward<KeyU>(key) - 1;

            if (prev > key)
            {
                throw std::runtime_error("-1 overflow");
            }

            return prev;
        }

        RightMap m_map;
    };
}
