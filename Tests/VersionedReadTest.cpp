#include "Awl/Io/Context.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Testing/UnitTest.h"

#include <iostream>
#include <algorithm>
#include <functional>

using namespace awl::testing;

struct A1
{
    int a;
    double b;
    std::string c;

    AWL_STRINGIZABLE(a, b, c)
};

AWL_MEMBERWISE_EQUATABLE(A1)

struct B1
{
    int x;
    double y;

    AWL_STRINGIZABLE(x, y)
};

AWL_MEMBERWISE_EQUATABLE(B1)

struct A2
{
    int a;
    double b;
    std::string c;
    int d = 5;

    AWL_STRINGIZABLE(a, b, c, d)
};

AWL_MEMBERWISE_EQUATABLE(A2)

struct B2
{
    int x;
    double y;
    std::string z;

    AWL_STRINGIZABLE(x, y, z)
};

AWL_MEMBERWISE_EQUATABLE(B2)

typedef std::variant<bool, char, int, float, double, std::string> FieldV;

typedef awl::io::Context<std::variant<A1, B1>, FieldV> OldContext;
typedef awl::io::Context<std::variant<A2, B2>, FieldV> NewContext;

AWT_TEST(Context)
{
    AWT_UNUSED_CONTEXT;

    std::vector<uint8_t> v;

    {
        awl::io::VectorOutputStream out(v);

        OldContext ctx;
        ctx.Initialize();
        
        auto a1 = ctx.MakeNewPrototype<A1>();
        Assert::IsTrue(a1.GetCount() == 3);

        auto b1 = ctx.MakeNewPrototype<B1>();
        Assert::IsTrue(b1.GetCount() == 2);

        ctx.WriteNew(out);
    }

    {
        awl::io::VectorInputStream in(v);

        NewContext ctx;
        ctx.ReadOld(in);

        auto a2 = ctx.MakeNewPrototype<A2>();
        Assert::IsTrue(a2.GetCount() == 4);

        auto b2 = ctx.MakeNewPrototype<B2>();
        Assert::IsTrue(b2.GetCount() == 3);

        auto & a1 = ctx.FindOldPrototype<A2>();
        Assert::IsTrue(a1.GetCount() == 3);

        auto & b1 = ctx.FindOldPrototype<B2>();
        Assert::IsTrue(b1.GetCount() == 2);
    }
}

AWT_TEST(VersionedRead)
{
    AWT_UNUSED_CONTEXT;
}
