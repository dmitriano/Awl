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
    double b;
    int d = 5;
    std::string e = "xyz";
    std::string c;

    AWL_STRINGIZABLE(b, d, e, c)
};

AWL_MEMBERWISE_EQUATABLE(A2)

struct B2
{
    int x;
    std::vector<int> z{ 1, 2, 3 };
    std::string w = "xyz";

    AWL_STRINGIZABLE(x, z, w)
};

AWL_MEMBERWISE_EQUATABLE(B2)

typedef std::variant<bool, char, int, float, double, std::string> FieldV1;
typedef std::variant<bool, char, int, float, double, std::string, std::vector<int>> FieldV2;

typedef awl::io::Context<std::variant<A1, B1>, FieldV1> OldContext;
typedef awl::io::Context<std::variant<A2, B2>, FieldV2> NewContext;

namespace awl::io
{
    template<class Stream, class Struct, class Context>
    inline void ReadV(Stream & s, Struct & val, const Context & ctx)
    {
        auto & new_proto = ctx.FindNewPrototype<Struct>();
        auto & old_proto = ctx.FindOldPrototype<Struct>();
        auto readers = ctx.MakeFieldReaders<Stream>();
        auto name_map = ctx.FindProtoMap<Struct>();

        assert(name_map.size() == old_proto.GetCount());

        for (size_t old_index = 0; old_index < name_map.size(); ++old_index)
        {
            const auto old_field = old_proto.GetField(old_index);
            auto reader = readers[old_field.type];
            //We read it even if it does not longer exist in the new struct.
            auto v = reader(s);

            const size_t new_index = name_map[old_index];

            if (new_index == Prototype::NoIndex)
            {
                if (!ctx.allowDelete)
                {
                    throw FieldNotFoundException(old_field.name);
                }
            }
            else
            {
                const auto new_field = new_proto.GetField(new_index);

                if (new_field.type != old_field.type)
                {
                    throw TypeMismatchException(new_field.name, new_field.type, old_field.type);
                }

                new_proto.Set(val, new_index, std::move(v));
            }
        }
    }
}

AWT_TEST(VersionedRead)
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
            auto & a1_proto = ctx.FindNewPrototype<A1>();
            Assert::IsTrue(a1_proto.GetCount() == 3);

            auto & b1_proto = ctx.FindNewPrototype<B1>();
            Assert::IsTrue(b1_proto.GetCount() == 2);
        }

        ctx.WriteNewPrototypes(out);

        Write(out, a1);
        Write(out, b1);
    }

    {
        awl::io::VectorInputStream in(v);

        NewContext ctx;
        ctx.ReadOldPrototypes(in);

        {
            auto & a2_proto = ctx.FindNewPrototype<A2>();
            Assert::IsTrue(a2_proto.GetCount() == 4);

            auto & b2_proto = ctx.FindNewPrototype<B2>();
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

        //Assert::IsTrue(a2.a == a1.a);
        Assert::IsTrue(a2.b == a1.b);
        Assert::IsTrue(a2.c == a1.c);
        Assert::IsTrue(a2.d == 5);

        Assert::IsTrue(b2.x == b1.x);
        //Assert::IsTrue(b2.y == b1.y);
    }
}
