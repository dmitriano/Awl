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
    std::vector<int> z;

    AWL_STRINGIZABLE(x, y, z)
};

AWL_MEMBERWISE_EQUATABLE(B2)

typedef std::variant<bool, char, int, float, double, std::string> FieldV1;
typedef std::variant<bool, char, int, float, double, std::string, std::vector<int>> FieldV2;

typedef awl::io::Context<std::variant<A1, B1>, FieldV1> OldContext;
typedef awl::io::Context<std::variant<A2, B2>, FieldV2> NewContext;

template<class Stream, class Struct, class Context>
inline void ReadV(Stream & s, Struct & val, const Context & ctx)
{
    auto new_proto = ctx.MakeNewPrototype<Struct>();
    auto & old_proto = ctx.FindOldPrototype<Struct>();
    auto readers = ctx.MakeFieldReaders<Stream>();

    for (int old_index = 0; old_index < old_proto.GetCount(); ++old_index)
    {
        const auto field = old_proto.GetField(old_index);
        auto reader = readers[field.type];
        auto v = reader(s);

    }
}

AWT_TEST(Context)
{
    AWT_UNUSED_CONTEXT;

    A1 a1 = { 1, 2.0, "abc" };
    B1 b1 = { 1, 2.0 };

    std::vector<uint8_t> v;

    {
        awl::io::VectorOutputStream out(v);

        OldContext ctx;
        ctx.Initialize();
        
        {
            auto a1_proto = ctx.MakeNewPrototype<A1>();
            Assert::IsTrue(a1_proto.GetCount() == 3);

            auto b1_proto = ctx.MakeNewPrototype<B1>();
            Assert::IsTrue(b1_proto.GetCount() == 2);
        }

        ctx.WriteNew(out);

        Write(out, a1);
        Write(out, b1);
    }

    {
        awl::io::VectorInputStream in(v);

        NewContext ctx;
        ctx.ReadOld(in);

        {
            auto a2_proto = ctx.MakeNewPrototype<A2>();
            Assert::IsTrue(a2_proto.GetCount() == 4);

            auto b2_proto = ctx.MakeNewPrototype<B2>();
            Assert::IsTrue(b2_proto.GetCount() == 3);

            auto & a1_proto = ctx.FindOldPrototype<A2>();
            Assert::IsTrue(a1_proto.GetCount() == 3);

            auto & b1_proto = ctx.FindOldPrototype<B2>();
            Assert::IsTrue(b1_proto.GetCount() == 2);
        }

        A2 a2;
        B2 b2;
        
        ReadV(in, a2, ctx);
        ReadV(in, b2, ctx);
    }
}

AWT_TEST(VersionedRead)
{
    AWT_UNUSED_CONTEXT;
}
