/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <type_traits>

namespace awl
{
    template <class T, class Field>
    class field_getter
    {
    public:

        using object_type = T;
        using value_type = Field;

        constexpr field_getter(Field T::* p) : m_p(p) {}

        //field_getter(const field_getter&) = default;
        //field_getter(field_getter&&) = default;
        //field_getter& operator = (const field_getter&) = default;
        //field_getter& operator = (field_getter&&) = default;

        constexpr const Field& operator() (const T & val) const
        {
            return val.*m_p;
        }
    
    // private:

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

        //func_getter(const func_getter&) = default;
        //func_getter(func_getter&&) = default;
        //func_getter& operator = (const func_getter&) = default;
        //func_getter& operator = (func_getter&&) = default;

        constexpr ReturnType operator() (const T& val) const
        {
            return (val.*m_p)();
        }

    // private:

        MyFuncPtr m_p;
    };

    template <class Getter, Getter state>
    class stateless_getter
    {
    public:

        constexpr typename Getter::value_type operator() (const typename Getter::object_type& object) const
        {
            return m_getter(object);
        }

    private:

        static inline constexpr Getter m_getter = state;
    };
}
