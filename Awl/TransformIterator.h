#pragma once

#include <iterator>
#include <type_traits>

namespace awl
{
    template <class UnaryFunction, class Iterator, class Value>
    class transform_iterator
    {
    public:

        transform_iterator(Iterator i, const UnaryFunction & func) : m_i(i), m_func(func)
        {
        }

        typedef std::forward_iterator_tag iterator_category;

        typedef typename std::remove_reference<Value>::type value_type;

        //typedef std::ptrdiff_t difference_type;

        typedef value_type * pointer;

        typedef value_type & reference;

        auto operator-> () const { return cur(); }

        auto operator* () const { return cur(); }

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
        auto cur() const { return m_func(*m_i);}

    private:

        Iterator m_i;

        const UnaryFunction & m_func;
    };

#if AWL_CPPSTD >= 17

    // For generic types, directly use the result of the signature of its 'operator()'
    template <typename T>
    struct function_traits : public function_traits<decltype(&T::operator())> {};

    //We specialize for pointers to member function.
    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType(ClassType::*)(Args...) const>
    {
        enum { arity = sizeof...(Args) };
        // arity is the number of arguments.

        typedef ReturnType result_type;

        template <size_t i>
        struct arg
        {
            typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
            // the i-th argument is equivalent to the i-th tuple element of a tuple
            // composed of those arguments.
        };
    };

    template <class Iterator, class UnaryFunction>
    inline auto make_transform_iterator(Iterator i, UnaryFunction func)
    {
        return transform_iterator<UnaryFunction, Iterator, awl::function_traits<UnaryFunction>::result_type>(i, func);
    }

#else

    template <typename T>
    struct return_type;
    template <typename R, typename... Args>
    struct return_type<R(*)(Args...)> { using type = R; };
    template <typename R, typename C, typename... Args>
    struct return_type<R(C::*)(Args...)> { using type = R; };
    template <typename R, typename C, typename... Args>
    struct return_type<R(C::*)(Args...) const> { using type = R; };
    template <typename R, typename C, typename... Args>
    struct return_type<R(C::*)(Args...) volatile> { using type = R; };
    template <typename R, typename C, typename... Args>
    struct return_type<R(C::*)(Args...) const volatile> { using type = R; };
    template <typename T>
    using return_type_t = typename return_type<T>::type;

#endif

    template <class Iterator, class UnaryFunction>
    inline auto make_transform_iterator(Iterator i, UnaryFunction func)
    {
        return transform_iterator<UnaryFunction, Iterator, awl::return_type_t<decltype(&UnaryFunction::operator())>>(i, func);
    }
}
