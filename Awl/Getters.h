/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Tuplizable.h"

#include <type_traits>

#if defined(__GNUC__)
#include <functional>
#endif

namespace awl
{
    // field_getter and func_getter can't be declared via std::mem_fn,
    // becuase second template parameter of std::mem_fn is not a type,
    // but a pointer to member value, so we can't do somethig like this:
    //
    // template <class T, class Field>
    // using field_getter = decltype(std::mem_fn(&T::Field));

    template <class T, class Field>
    class field_getter
    {
    public:

        using object_type = T;
        using value_type = Field;

        constexpr field_getter(Field T::* p) : m_p(p) {}

        constexpr const Field& operator() (const T & val) const
        {
            return val.*m_p;
        }
    
    private:

        Field T::* m_p;
    };
    
    template <class T, class ReturnType>
    using FuncPtr = ReturnType(T::*)() const;

    template <class T, class ReturnType>
    class func_getter
    {
    public:

        using object_type = T;
        using value_type = ReturnType;

        using MyFuncPtr = FuncPtr<T, ReturnType>;

        constexpr func_getter(MyFuncPtr p) : m_p(p) {}

        constexpr ReturnType operator() (const T& val) const
        {
            return (val.*m_p)();
        }

    private:

        MyFuncPtr m_p;
    };

    template <class T, class Field>
    field_getter<T, Field> make_getter(Field T::* p)
    {
        return field_getter<T, Field>{ p };
    }

    template <class T, class ReturnType>
    func_getter<T, ReturnType> make_getter(FuncPtr<T, ReturnType> p)
    {
        return func_getter<T, ReturnType>{ p };
    }

    template <class T, size_t index>
    struct tuple_element_getter
    {
        std::tuple_element_t<index, T> operator() (const T& val) const
        {
            return std::get<index>(val);
        }
    };

    template <class T, size_t index>
    struct tuplizable_getter
    {
        std::tuple_element_t<index, typename tuplizable_traits<T>::ConstTie> operator() (const T& val) const
        {
            return std::get<index>(object_as_const_tuple(val));
        }
    };

    // Stateless geters:

    template <auto value>
    class getter;

// GCC12 BUG: Field T::* is less specialized than ReturnType(T::* func_ptr)() const
#if defined(__GNUC__)

    template <class T, class Field, Field T::* field_ptr>
    class getter<field_ptr>
    {
    public:

        using object_type = T;
        using value_type = Field;

        constexpr decltype(auto) operator() (const T& val) const
        {
            return std::invoke(field_ptr, val);
        }
    };

#else

    template <class T, class ReturnType, ReturnType(T::* func_ptr)() const>
    class getter<func_ptr>
    {
    public:

        using object_type = T;
        using value_type = ReturnType;

        constexpr ReturnType operator() (const T& val) const
        {
            return (val.*func_ptr)();
        }
    };

    template <class T, class Field, Field T::* field_ptr>
    class getter<field_ptr>
    {
    public:

        using object_type = T;
        using value_type = Field;

        constexpr const Field& operator() (const T& val) const
        {
            return val.*field_ptr;
        }
    };

#endif
}
