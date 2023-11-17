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

    // Stateless geters:

    template <auto value>
    class getter;

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
}
