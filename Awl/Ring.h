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

namespace awl
{
    template <class T, class Allocator = std::allocator<T>>
    class ring
    {
    public:

        using value_type = T;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = T &;
        using const_reference = const T &;
        using pointer = T *;
        using const_pointer = const T *;

    private:

        template <class E>
        class ring_iterator
        {
        public:

            using iterator_category = std::random_access_iterator_tag;
            using value_type = E;
            using difference_type = std::ptrdiff_t;
            using reference = E &;
            using pointer = E *;

            ring_iterator() : m_pRing(nullptr), m_pos(0) {}
            
            ring_iterator(const ring_iterator& other) = default;
            ring_iterator(ring_iterator&& other) = default;

            ring_iterator& operator = (const ring_iterator& other) = default;
            ring_iterator& operator = (ring_iterator&& other) = default;

            pointer operator-> () const { return container().template address<E>(m_pos); }

            reference operator* () const { return *container().template address<E>(m_pos); }

            ring_iterator & operator++ ()
            {
                move_next();

                return *this;
            }

            ring_iterator operator++ (int)
            {
                ring_iterator tmp = *this;

                move_next();

                return tmp;
            }

            ring_iterator & operator-- ()
            {
                move_prev();

                return *this;
            }

            ring_iterator operator-- (int)
            {
                ring_iterator tmp = *this;

                move_prev();

                return tmp;
            }

            ring_iterator & operator += (difference_type diff)
            {
                m_pos += diff;

                return *this;
            }

            ring_iterator & operator -= (difference_type diff)
            {
                return this->operator+=(-diff);
            }

            ring_iterator operator + (difference_type diff) const
            {
                ring_iterator i(container(), m_pos + diff);
                
                return i;
            }

            ring_iterator operator - (difference_type diff) const
            {
                return this->operator+(-diff);
            }

            difference_type operator - (const ring_iterator & other) const
            {
                return position() - other.position();
            }

            bool operator == (const ring_iterator & other) const
            {
                return m_pos == other.m_pos;
            }

            bool operator != (const ring_iterator & other)  const
            {
                return !(*this == other);
            }

            bool operator < (const ring_iterator & other) const
            {
                return m_pos < other.m_pos;
            }

            operator ring_iterator<const E>() const
            {
                return ring_iterator<const E>(container(), m_pos);
            }

        private:

            ring_iterator(const ring & r, std::size_t pos) : m_pRing(&r), m_pos(pos)
            {
            }

            void move_next()
            {
                ++m_pos;
            }

            void move_prev()
            {
                --m_pos;
            }

            const ring& container() const
            {
                return *m_pRing;
            }

            difference_type position() const
            {
                return static_cast<difference_type>(m_pos);
            }

            const ring * m_pRing;
            
            //It can't be a pointer because there is no
            //end pointer in a circular buffer, so we use an index.
            std::size_t m_pos;

            friend ring;
        };

    public:

        using iterator = ring_iterator<T>;
        using const_iterator = ring_iterator<const T>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        ring(Allocator alloc = {}) : m_alloc(alloc),
            m_buf(nullptr)
        {
        }

        ring(size_type cap, Allocator alloc = {}) : m_alloc(alloc),
            m_buf(m_alloc.allocate(cap)), m_capacity(cap),
            m_data(m_buf), m_size(0)
        {
            assert(cap != 0);
        }

        ring(const ring & other)
        {
            copy(other);
        }

        ring(ring && other)
        {
            attach(other);

            other.release();
        }

        ring & operator = (const ring & other)
        {
            //Not an assignment to itself.
            if (m_buf != other.m_buf)
            {
                free();

                copy(other);
            }

            return *this;
        }

        ring & operator = (ring && other)
        {
            //Not an assignment to itself.
            if (m_buf != other.m_buf)
            {
                free();

                attach(other);

                other.release();
            }

            return *this;
        }

        ~ring()
        {
            free();
        }

        void reserve(size_type cap)
        {
            assert(cap != 0);

            T * buf = m_alloc.allocate(cap);

            size_type min_size;
            
            if (m_buf != nullptr)
            {
                min_size = std::min(cap, size());

                for (size_type i = 0; i != min_size; ++i)
                {
                    new (buf + i) T(std::move(operator[](size() - min_size + i)));
                }

                free();
            }
            else
            {
                min_size = 0;
            }

            m_buf = buf;
            m_capacity = cap;

            m_data = m_buf;
            m_size = min_size;
        }

        void clear()
        {
            assert(m_buf != nullptr);

            while(!empty())
            {
                pop_front();
            }

            m_data = m_buf;
            m_size = 0;
        }

        reference front() { assert(!empty()); return *m_data; }
        reference back() { assert(!empty()); return *last<T>(); }

        const_reference front() const { assert(!empty()); return *m_data; }
        const_reference back() const { assert(!empty()); return *last<const T>();}

        void push_front(const value_type & val)
        {
            new (allocate_prev()) T(val);
        }

        void push_front(value_type && val)
        {
            new (allocate_prev()) T(std::move(val));
        }

        void push_back(const value_type & val)
        {
            new (allocate_next()) T(val);
        }

        void push_back(value_type && val)
        {
            new (allocate_next()) T(std::move(val));
        }

        void pop_front()
        {
            assert(!empty());

            m_data->~T();
            
            m_data = next(m_data);

            --m_size;
        }

        void pop_back()
        {
            assert(!empty());

            last<T>()->~T();

            --m_size;
        }

        size_type size() const
        {
            return m_size;
        }
        
        size_type capacity() const
        {
            return m_capacity;
        }
        
        bool empty() const
        {
            return m_size == 0;
        }
        
        bool full() const
        {
            return size() == capacity();
        }

        reference operator[](size_type index)
        {
            return *address<T>(index);
        }

        const_reference operator[](size_type index) const
        {
            return *address<const T>(index);
        }

        reference at(size_type index)
        {
            check_index(index);

            return operator [](index);
        }

        const_reference at(size_type index) const
        {
            check_index(index);

            return operator [](index);
        }

        iterator begin() { return ring_iterator<T>(*this, 0u); }
        const_iterator begin() const { return cbegin(); }
        const_iterator cbegin() const { return ring_iterator<const T>(*this, 0u); }

        iterator end() { return ring_iterator<T>(*this, m_size); }
        const_iterator end() const { return cend(); }
        const_iterator cend() const { return ring_iterator<const T>(*this, m_size);}

        reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }
        const_reverse_iterator rbegin() const { return crbegin(); }
        const_reverse_iterator crbegin() const { return std::make_reverse_iterator(cend()); }

        reverse_iterator rend() { return std::make_reverse_iterator(begin()); }
        const_reverse_iterator rend() const { return crend(); }
        const_reverse_iterator crend() const { return std::make_reverse_iterator(cbegin()); }

    private:

        void check_index(std::size_t index) const
        {
            if (index > size())
            {
                throw std::out_of_range("ring index is out of range");
            }
        }
        
        //We do not know the direction the pointer was moved to,
        //because 'diff' is a signed integer, so we check both begin and end.
        template <class E>
        void adjust(E *& p) const
        {
            if (!adjust_underflow(p))
            {
                adjust_overflow(p);
            }
        }

        template <class E>
        bool adjust_underflow(E *& p) const
        {
            if (p < m_buf)
            {
                const difference_type diff = m_buf - p;

                p = buf_end() - diff;

                return true;
            }

            return false;
        }

        template <class E>
        bool adjust_overflow(E *& p) const
        {
            if (p >= buf_end())
            {
                const difference_type diff = p - buf_end();

                p = m_buf + diff;

                return true;
            }

            return false;
        }

        template <class E>
        E * address(std::size_t pos) const
        {
            E * p = m_data + pos;

            adjust_overflow(p);

            return p;
        }

        T * buf_end() const
        {
            return m_buf + capacity();
        }

        template <class E>
        E * last() const
        {
            return address<E>(size() - 1);
        }

        //The address where we write the next element.
        //If the buffer is full it is equal to m_data.
        T * data_end() const
        {
            return address<T>(size());
        }

        T * next(T * p) const
        {
            T * p_next = p;

            ++p_next;
            
            if (p_next == buf_end())
            {
                return m_buf;
            }

            return p_next;
        }

        T * prev(T * p) const
        {
            T * p_prev = p;

            if (p_prev == m_buf)
            {
                p_prev = buf_end();
            }
            
            --p_prev;

            return p_prev;
        }

        T * allocate_next()
        {
            T * const p_write = data_end();

            if (full())
            {
                assert(m_data == p_write);

                m_data->~T();

                m_data = next(m_data);
            }
            else
            {
                ++m_size;
            }

            return p_write;
        }

        T * allocate_prev()
        {
            const bool saved_full = full();
            
            m_data = prev(m_data);

            if (saved_full)
            {
                m_data->~T();
            }
            else
            {
                ++m_size;
            }

            return m_data;
        }

        void free()
        {
            if (m_buf != nullptr)
            {
                clear();

                m_alloc.deallocate(m_buf, capacity());

                m_buf = nullptr;
            }
        }

        void release()
        {
            m_buf = nullptr;
        }

        void attach(const ring & other)
        {
            m_capacity = other.m_capacity;
            m_buf = other.m_buf;

            m_data = other.m_data;
            m_size = other.m_size;
        }

        void copy(const ring & other)
        {
            m_capacity = other.m_capacity;
            m_buf = m_alloc.allocate(m_capacity);

            m_data = m_buf;
            m_size = 0;

            for (const T & val : other)
            {
                push_back(val);
            }
        }

        Allocator m_alloc;

        T * m_buf;
        std::size_t m_capacity;

        T * m_data;
        std::size_t m_size;

        template <class E>
        friend class ring_iterator;
    };
}
