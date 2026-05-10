/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/UnitTest.h"

#include <stdlib.h>
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
    MyAllocator<64> a;

    context.logger->debug("allocated a.data at {} ({} bytes)", static_cast<void*>(a.data), sizeof a.data);

    // Allocate A
    if (A* p = a.implicit_aligned_alloc<A>())
    {
        context.logger->debug("allocated A char at {}", static_cast<void*>(p));
        context.logger->debug("Original value: {:x}", p->x);
        *p = {};
        context.logger->debug("Default value: {:x}", p->x);
        p->x = 0x42;
        context.logger->debug("Updated value: {:x}", p->x);
    }

    // Allocate a char
    if (char* p = a.implicit_aligned_alloc<char>())
    {
        *p = 'a';
        context.logger->debug("allocated a char at {}", static_cast<void*>(p));
    }

    // Allocate an int
    if (int* p = a.implicit_aligned_alloc<int>())
    {
        *p = 1;
        context.logger->debug("allocated an int at {}", static_cast<void*>(p));
    }

    // Allocate an int, aligned at a 32-byte boundary
    if (int* p = a.implicit_aligned_alloc<int>(32))
    {
        *p = 2;
        context.logger->debug("allocated an int at {} (32-byte alignment)", static_cast<void*>(p));
    }
}

namespace
{
    struct B
    {
        int x;
        char y;

        int64_t z[10];

        char y1;
    };
}

AWL_EXAMPLE(ImplicitLifetimeVector)
{
    AWL_ATTRIBUTE(size_t, offset, 0);

    context.logger->debug(_T("sizeof(B): {}"), sizeof(B));

    std::vector<uint8_t> v(sizeof(B) + offset, 0);

    B* p = reinterpret_cast<B*>(v.data() + offset);

    p->x = 1;
    p->y = 'a';

    context.logger->debug(_T("{} {} {}"), p->x, p->y, v[offset + 4]);

    v[offset + 0] = 10;
    v[offset + 4] = 66;

    context.logger->debug(_T("{} {}"), p->x, p->y);
}

AWL_EXAMPLE(ImplicitLifetimeCalloc)
{
    AWL_ATTRIBUTE(size_t, offset, 0);

    context.logger->debug(_T("sizeof(B): {}, alignof(B): {}"), sizeof(B), alignof(B));

    void* p1 = calloc(1, sizeof(B) + offset);

    B* p = reinterpret_cast<B*>(reinterpret_cast<char*>(p1) + offset);

    p->x = 1;
    p->y = 'a';

    context.logger->debug(_T("{} {}"), p->x, p->y);

    free(p1);
}

AWL_EXAMPLE(ImplicitLifetimePlacementNew)
{
    struct S { int x; char c; };

    alignas(S) char buf[sizeof(S)];
    S* p = new(buf) S{};

    p->x = 10;
    p->c = 'a';

    context.logger->debug(_T("{} {}"), p->x, buf[4]);

    // An object of type char, unsigned char, or std::byte may be used to access the object representation of any object.
    // A char, unsigned char, or std::byte array may be used to manipulate the object representation of any object.
    buf[4] = 'b';

    context.logger->debug(_T("{} {}"), p->x, p->c);
}
