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

        using MyFuncPtr = FuncPtr<T, ReturnType>;

        constexpr func_getter(MyFuncPtr p) : m_p(p) {}

        constexpr ReturnType operator() (const T& val) const
        {
            return (val.*m_p)();
        }

    private:

        MyFuncPtr m_p;
    };
}
