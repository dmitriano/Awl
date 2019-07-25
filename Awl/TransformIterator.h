#pragma once

#include "Awl/FunctionTraits.h"

#include <iterator>
#include <type_traits>

namespace awl
{
    template <class UnaryFunction, class Iterator, class Container = void>
    class transform_iterator
    {
    public:

        transform_iterator(Iterator i, const UnaryFunction & func) : m_i(i), m_func(func)
        {
        }

#if AWL_CPPSTD >= 17

        typedef typename awl::function_traits<UnaryFunction>::result_type Value;
        
#else

        typedef awl::return_type_t<decltype(&UnaryFunction::operator())> Value;

#endif

        typedef std::forward_iterator_tag iterator_category;

        typedef typename std::remove_reference<Value>::type value_type;

        typedef std::ptrdiff_t difference_type;

        typedef value_type * pointer;

        typedef value_type & reference;

        Value operator-> () const { return cur(); }

        Value operator* () const { return cur(); }

        transform_iterator & operator++ ()
        {
            ++m_i;

            return *this;
        }

        transform_iterator operator++ (int)
        {
            transform_iterator tmp = *this;

            ++m_i;

            return tmp;
        }

        bool operator == (const transform_iterator & r) const
        {
            return m_i == r.m_i;
        }

        bool operator != (const transform_iterator & r)  const
        {
            return m_i != r.m_i;
        }

    protected:

        //m_func may return a reference or a value.
        Value cur() const { return m_func(*m_i);}

    private:

        Iterator m_i;

        UnaryFunction m_func;

        friend Container;
    };

    template <class Iterator, class UnaryFunction>
    inline auto make_transform_iterator(Iterator i, UnaryFunction func)
    {
        return transform_iterator<UnaryFunction, Iterator>(i, func);
    }

    template <class Container, class Iterator, class UnaryFunction>
    inline auto make_friend_iterator(Iterator i, UnaryFunction func)
    {
        return transform_iterator<UnaryFunction, Iterator, Container>(i, func);
    }
}
