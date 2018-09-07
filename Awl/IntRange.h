#pragma once

#include "Awl/Range.h"

#include <type_traits>
#include <iterator>

namespace awl
{
    template <typename T, typename Enable = void>
    class int_iterator;

    template <typename T>
    class int_iterator<T, typename std::enable_if<std::is_integral<T>::value>::type>
    {
    public:

        explicit int_iterator(T n) : m_n(n)
        {
        }

        typedef std::forward_iterator_tag iterator_category;

        typedef T value_type;

        typedef std::ptrdiff_t difference_type;

        typedef value_type * pointer;

        typedef value_type & reference;

        value_type operator-> () const { return cur(); }

        value_type operator* () const { return cur(); }

        int_iterator & operator++ ()
        {
            ++m_n;

            return *this;
        }

        int_iterator operator++ (int)
        {
            int_iterator tmp = *this;

            ++m_n;

            return tmp;
        }

        bool operator == (const int_iterator & r) const
        {
            return m_n == r.m_n;
        }

        bool operator != (const int_iterator & r)  const
        {
            return m_n != r.m_n;
        }

    protected:

        value_type cur() const { return m_n;}

    private:

        T m_n;
    };

    template <class T>
    inline auto make_int_iterator(T n)
    {
        return int_iterator<T>(n);
    }

    template <class T>
    inline auto make_int_range(T begin, T end)
    {
        return make_range(int_iterator<T>(begin), int_iterator<T>(end));
    }

    template <class T>
    inline auto make_count(T end)
    {
        return make_int_range(static_cast<T>(0), end);
    }
}
