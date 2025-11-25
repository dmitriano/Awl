/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <format>
#include <stdlib.h>

#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

class VirtualClass
{
public:
    virtual ~VirtualClass() = default;
};

class VirtualClass2
{
public:
    virtual ~VirtualClass2() = default;
};

class DerivedVirtualClass : public VirtualClass
{
public:
    virtual ~DerivedVirtualClass() = default;
};

class Derived2VirtualClass
    : public VirtualClass
    , public VirtualClass2
{
public:
    virtual ~Derived2VirtualClass() = default;
};

template <class T>
concept IsInterface =
std::is_class_v<T> && not std::is_final_v<T> && std::has_virtual_destructor_v<T>
&& (sizeof(T) == sizeof(VirtualClass) || sizeof(T) == sizeof(DerivedVirtualClass)
    || sizeof(T) == sizeof(Derived2VirtualClass));

namespace
{
    class A
    {
    public:

        virtual ~A() = default;
    };

    class B : public A {};

    class C : public A {};

    class D : public B, public C {};

    class E : public B, public C {};

    class F : public D, public E {};

    class A1
    {
    public:

        virtual ~A1() = default;
    };

    class X : public F, public A1 {};

    // static_assert(IsInterface<D>);
}

AWL_EXAMPLE(Interface)
{
    std::cout << "sizeof(A): " << sizeof(A) << std::endl;
    std::cout << "sizeof(B): " << sizeof(B) << std::endl;
    std::cout << "sizeof(C): " << sizeof(C) << std::endl;
    std::cout << "sizeof(D): " << sizeof(D) << std::endl;
    std::cout << "sizeof(E): " << sizeof(E) << std::endl;
    std::cout << "sizeof(X): " << sizeof(E) << std::endl;
}
