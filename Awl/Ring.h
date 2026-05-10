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
#include <ranges>
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

            ring_iterator() : _pRing(nullptr), _pos(0) {}
            
            ring_iterator(const ring_iterator& other) = default;
            ring_iterator(ring_iterator&& other) = default;

            ring_iterator& operator = (const ring_iterator& other) = default;
            ring_iterator& operator = (ring_iterator&& other) = default;

            pointer operator-> () const { return container().template address<E>(_pos); }

            reference operator* () const { return *container().template address<E>(_pos); }

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
                _pos += diff;

                return *this;
            }

            ring_iterator & operator -= (difference_type diff)
            {
                return this->operator+=(-diff);
            }

            ring_iterator operator + (difference_type diff) const
            {
                ring_iterator i(container(), _pos + diff);
                
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
                return _pos == other._pos;
            }

            bool operator != (const ring_iterator & other)  const
            {
                return !(*this == other);
            }

            bool operator < (const ring_iterator & other) const
            {
                return _pos < other._pos;
            }

            operator ring_iterator<const E>() const
            {
                return ring_iterator<const E>(container(), _pos);
            }

        private:

            ring_iterator(const ring & r, std::size_t pos) : _pRing(&r), _pos(pos)
            {}

            void move_next()
            {
                ++_pos;
            }

            void move_prev()
            {
                --_pos;
            }

            const ring& container() const
            {
                return *_pRing;
            }

            difference_type position() const
            {
                return static_cast<difference_type>(_pos);
            }

            const ring * _pRing;
            
            //It can't be a pointer because there is no
            //end pointer in a circular buffer, so we use an index.
            std::size_t _pos;

            friend ring;
        };

    public:

        using iterator = ring_iterator<T>;
        using const_iterator = ring_iterator<const T>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        ring(Allocator alloc = {}) : _alloc(alloc),
            _buf(nullptr)
        {}

        ring(size_type cap, Allocator alloc = {}) : _alloc(alloc),
            _buf(_alloc.allocate(cap)), _capacity(cap),
            _data(_buf), _size(0)
        {
            assert(cap != 0);
        }

        ring(const ring & other)
        {
            copy(other);
        }

        ring(ring && other) noexcept
        {
            attach(other);

            other.release();
        }

        ring & operator = (const ring & other)
        {
            //Not an assignment to itself.
            if (_buf != other._buf)
            {
                free();

                copy(other);
            }

            return *this;
        }

        ring & operator = (ring && other) noexcept
        {
            //Not an assignment to itself.
            if (_buf != other._buf)
            {
                free();

                attach(other);

                other.release();
            }

            return *this;
        }

        bool operator == (const ring& other) const noexcept
        {
            return std::ranges::equal(*this, other);
        }

        ~ring()
        {
            free();
        }

        void reserve(size_type cap)
        {
            assert(cap != 0);

            T * buf = _alloc.allocate(cap);

            size_type min_size;
            
            if (_buf != nullptr)
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

            _buf = buf;
            _capacity = cap;

            _data = _buf;
            _size = min_size;
        }

        void clear()
        {
            assert(_buf != nullptr);

            while(!empty())
            {
                pop_front();
            }

            _data = _buf;
            _size = 0;
        }

        reference front() { assert(!empty()); return *_data; }
        reference back() { assert(!empty()); return *last<T>(); }

        const_reference front() const { assert(!empty()); return *_data; }
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

            _data->~T();
            
            _data = next(_data);

            --_size;
        }

        void pop_back()
        {
            assert(!empty());

            last<T>()->~T();

            --_size;
        }

        size_type size() const
        {
            return _size;
        }
        
        size_type capacity() const
        {
            return _capacity;
        }
        
        bool empty() const
        {
            return _size == 0;
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

        iterator end() { return ring_iterator<T>(*this, _size); }
        const_iterator end() const { return cend(); }
        const_iterator cend() const { return ring_iterator<const T>(*this, _size);}

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
            if (p < _buf)
            {
                const difference_type diff = _buf - p;

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

                p = _buf + diff;

                return true;
            }

            return false;
        }

        template <class E>
        E * address(std::size_t pos) const
        {
            E * p = _data + pos;

            adjust_overflow(p);

            return p;
        }

        T * buf_end() const
        {
            return _buf + capacity();
        }

        template <class E>
        E * last() const
        {
            return address<E>(size() - 1);
        }

        //The address where we write the next element.
        //If the buffer is full it is equal to _data.
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
                return _buf;
            }

            return p_next;
        }

        T * prev(T * p) const
        {
            T * p_prev = p;

            if (p_prev == _buf)
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
                assert(_data == p_write);

                _data->~T();

                _data = next(_data);
            }
            else
            {
                ++_size;
            }

            return p_write;
        }

        T * allocate_prev()
        {
            const bool saved_full = full();
            
            _data = prev(_data);

            if (saved_full)
            {
                _data->~T();
            }
            else
            {
                ++_size;
            }

            return _data;
        }

        void free()
        {
            if (_buf != nullptr)
            {
                clear();

                _alloc.deallocate(_buf, capacity());

                _buf = nullptr;
            }
        }

        void release()
        {
            _buf = nullptr;
        }

        void attach(const ring & other)
        {
            _capacity = other._capacity;
            _buf = other._buf;

            _data = other._data;
            _size = other._size;
        }

        void copy(const ring & other)
        {
            _capacity = other._capacity;
            _buf = _alloc.allocate(_capacity);

            _data = _buf;
            _size = 0;

            for (const T & val : other)
            {
                push_back(val);
            }
        }

        Allocator _alloc;

        T * _buf;
        std::size_t _capacity;

        T * _data;
        std::size_t _size;

        template <class E>
        friend class ring_iterator;
    };
}
