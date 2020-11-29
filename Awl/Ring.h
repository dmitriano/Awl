#pragma once

#include <stdexcept>
#include <cassert>
#include <initializer_list>

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

            ring_iterator & operator = (const ring_iterator & other)
            {
                m_p = other.m_p;

                return *this;
            }

            pointer operator-> () const { return m_p; }

            reference operator* () const { return *m_p; }

            ring_iterator & operator++ ()
            {
                move_next();

                return *this;
            }

            ring_iterator operator++ (int)
            {
                my_iterator tmp = *this;

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
                my_iterator tmp = *this;

                move_prev();

                return tmp;
            }

            ring_iterator & operator += (difference_type diff)
            {
                m_p += diff;

                adjust();

                return *this;
            }

            ring_iterator & operator -= (difference_type diff)
            {
                return this->operator+=(-diff);
            }

            ring_iterator operator + (difference_type diff)
            {
                ring_iterator i(m_ring, m_p + diff);
                
                i.adjust();

                return i;
            }

            ring_iterator operator - (difference_type diff)
            {
                return this->operator+(-diff);
            }

            difference_type operator - (const ring_iterator & other)
            {
                return position() - other.position();
            }

            bool operator == (const ring_iterator & other) const
            {
                return m_p == other.m_p;
            }

            bool operator != (const ring_iterator & other)  const
            {
                return !(*this == other);
            }

            bool operator < (const ring_iterator & other) const
            {
                return position() < other.position();
            }

            operator ring_iterator<const E>() const
            {
                return ring_iterator<const E>(m_ring, m_p);
            }

        private:

            ring_iterator(const ring & r, E * p) : m_ring(r), m_p(p)
            {
            }

            void move_next()
            {
                m_p = m_ring.next(m_p);
            }

            void move_prev()
            {
                m_p = m_ring.prev(m_p);
            }

            //We do not know the direction the pointer was moved to,
            //because 'diff' is a signed integer, so we check both begin and end.
            void adjust()
            {
                m_ring.adjust(m_p);
            }

            difference_type position() const
            {
                return m_ring.position(m_p);
            }

            const ring & m_ring;
            
            E * m_p;

            friend ring;
        };

    public:

        using iterator = ring_iterator<T>;
        using const_iterator = ring_iterator<const T>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        ring(Allocator alloc = {}) : m_alloc(alloc)
        {
        }

        ring(size_type cap, Allocator alloc = {}) : m_alloc(alloc),
            bufBegin(m_alloc.allocate(cap)), bufEnd(bufBegin + cap),
            dataBegin(bufBegin), dataEnd(dataBegin)
        {
        }

        ~ring()
        {
            clear();
            
            m_alloc.deallocate(bufBegin, capacity());
        }

        void reserve(size_type new_cap)
        {
            T * buf = m_alloc.allocate(cap);

            size_type min_size;
            
            if (bufBegin != nullptr)
            {
                size_type min_size = std::min(cap, size());

                for (size_type i = 0; ++i; i < min_size)
                {
                    buf[i] = std::move(bufBegin[i]);
                }

                m_alloc.deallocate(bufBegin, capacity());
            }
            else
            {
                min_size = 0;
            }

            bufBegin = buf;
            bufEnd = bufBegin + cap;

            dataBegin = bufBegin;
            dataEnd = dataBegin + min_size;
        }

        void clear()
        {
            assert(bufBegin != nullptr);

            for (T & elem : *this)
            {
                delete &elem;
            }

            dataBegin = bufBegin;
            dataEnd = dataBegin;
        }

        reference front() { assert(!empty()); return *dataBegin; }
        reference back() { assert(!empty()); return *prev(dataEnd); }

        const_reference front() const { assert(!empty()); return *dataBegin; }
        const_reference back() const { assert(!empty()); return *prev(dataEnd); }

        void push_back(const value_type & val)
        {
            *allocate_next() = val;
        }

        void push_back(value_type && val)
        {
            *allocate_next() = std::move(val);
        }

        void push(const value_type & val) { push_back(val); }
        void push(value_type && val) { push_back(val); }

        void pop_front()
        {
            assert(!empty());

            dataBegin = next(dataBegin);
        }

        void pop() { pop_front(); }
        
        size_type size() const
        {
            return static_cast<size_type>(position(dataEnd));
        }
        
        size_type capacity() const
        {
            return static_cast<size_type>(bufEnd - bufBegin);
        }
        
        bool empty() const
        {
            return dataBegin == dataEnd;
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
            if (index > size())
            {
                throw std::out_of_range("ring index is out of range");
            }
            
            return *address<T>(static_cast<difference_type>(index));
        }

        const_reference at(size_type index) const
        {
            if (index > size())
            {
                throw std::out_of_range("ring index is out of range");
            }

            return *address<const T>(static_cast<difference_type>(index));
        }

        iterator begin() { return ring_iterator(*this, dataBegin); }
        const_iterator begin() const { return cbegin(); }
        const_iterator cbegin() const { return ring_iterator(*this, dataBegin); }

        iterator end() { return ring_iterator(*this, dataEnd); }
        const_iterator end() const { return cend(); }
        const_iterator cend() const { return ring_iterator(*this, dataEnd);}

        reverse_iterator rbegin() { return std::make_reverse_iterator(begin()); }
        const_reverse_iterator rbegin() const { return crbegin(); }
        const_reverse_iterator crbegin() const { return std::make_reverse_iterator(cbegin()); }

        reverse_iterator rend() { return std::make_reverse_iterator(end()); }
        const_reverse_iterator rend() const { return crend(); }
        const_reverse_iterator crend() const { return std::make_reverse_iterator(cend()); }

    private:

        template <class E>
        E * next(E * p) const
        {
            if (p == bufEnd)
            {
                return bufBegin;
            }

            return p + 1;
        }

        template <class E>
        E * prev(E * p) const
        {
            if (p == bufBegin)
            {
                return bufEnd - 1;
            }

            return p - 1;
        }

        //We do not know the direction the pointer was moved to,
        //because 'diff' is a signed integer, so we check both begin and end.
        template <class E>
        void adjust(E *& p) const
        {
            if (p < bufBegin)
            {
                const difference_type diff = bufBegin - p;

                p = bufEnd - diff;
            }
            else if (p >= bufEnd)
            {
                const difference_type diff = p - bufEnd;

                p = bufBegin + diff;
            }
        }

        template <class E>
        difference_type position(E * p) const
        {
            const size_type pos = p - dataBegin;

            if (pos >= 0)
            {
                return pos;
            }

            return (p - bufBegin) + (bufEnd - 1 - dataBegin);
        }

        template <class E>
        E * address(difference_type pos)
        {
            E * p = dataBegin + pos;

            adjust(p);

            return p;
        }

        T * allocate_next()
        {
            T * p_write = dataEnd == bufEnd ? bufBegin : dataEnd;

            if (dataBegin == p_write)
            {
                pop_front();
            }

            dataEnd = next(p_write);

            return p_write;
        }

        Allocator m_alloc;

        T * bufBegin = nullptr;
        T * bufEnd = nullptr;

        T * dataBegin = nullptr;
        T * dataEnd = nullptr;

        template <class E>
        friend class ring_iterator;
    };
}
