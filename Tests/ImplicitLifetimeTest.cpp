/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <format>
#include <stdlib.h>

#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

namespace
{
    template<std::size_t N>
    struct MyAllocator
    {
        MyAllocator()
        {
            for (std::byte& b : data)
                b = std::byte{ 3 };
        }

        std::byte data[N];
        std::size_t sz{ N };
        void* p{ data };

        // Note: only well-defined for implicit-lifetime types
        template<typename T>
        T* implicit_aligned_alloc(std::size_t a = alignof(T))
        {
            if (std::align(a, sizeof(T), p, sz))
            {
                T* result1 = reinterpret_cast<T*>(p);
                T* result2 = std::launder(result1);
                p = static_cast<std::byte*>(p) + sizeof(T);
                sz -= sizeof(T);
                return result2;
            }
            return nullptr;
        }
    };

    struct A { int x; };
}

AWL_EXAMPLE(ImplicitLifetimeAllocator)
{
    AWL_UNUSED_CONTEXT;

    MyAllocator<64> a;

    std::cout << "allocated a.data at " << (void*)a.data
        << " (" << sizeof a.data << " bytes)\n";

    // Allocate A
    if (A* p = a.implicit_aligned_alloc<A>())
    {
        std::cout << "allocated A char at " << (void*)p << '\n';
        std::cout << "\tOriginal value: " << std::format("{:x}", p->x) << '\n';
        *p = {};
        std::cout << "\tDefault value: " << std::format("{:x}", p->x) << '\n';
        p->x = 0x42;
        std::cout << "\tUpdated value: " << std::format("{:x}", p->x) << '\n';
    }

    // Allocate a char
    if (char* p = a.implicit_aligned_alloc<char>())
    {
        *p = 'a';
        std::cout << "allocated a char at " << (void*)p << '\n';
    }

    // Allocate an int
    if (int* p = a.implicit_aligned_alloc<int>())
    {
        *p = 1;
        std::cout << "allocated an int at " << (void*)p << '\n';
    }

    // Allocate an int, aligned at a 32-byte boundary
    if (int* p = a.implicit_aligned_alloc<int>(32))
    {
        *p = 2;
        std::cout << "allocated an int at " << (void*)p << " (32-byte alignment)\n";
    }
}

namespace
{
    struct B
    {
        int x;
        char y;
    };
}

AWL_EXAMPLE(ImplicitLifetimeVector)
{
    AWL_ATTRIBUTE(size_t, offset, 0);

    context.logger.debug(awl::format() << "sizeof(B): " << sizeof(B));

    std::vector<uint8_t> v(sizeof(B) + offset, 0);

    B* p = reinterpret_cast<B*>(v.data() + offset);

    p->x = 1;
    p->y = 'a';

    context.logger.debug(awl::format() << p->x << " " << p->y);

    v[offset + 0] = 10;
    v[offset + 4] = 66;

    context.logger.debug(awl::format() << p->x << " " << p->y);
}

AWL_EXAMPLE(ImplicitLifetimeCalloc)
{
    AWL_ATTRIBUTE(size_t, offset, 0);

    context.logger.debug(awl::format() << "sizeof(B): " << sizeof(B));

    void* p1 = calloc(1, sizeof(B) + offset);

    B* p = reinterpret_cast<B*>(reinterpret_cast<char*>(p1) + offset);

    p->x = 1;
    p->y = 'a';

    context.logger.debug(awl::format() << p->x << " " << p->y);

    free(p1);
}
