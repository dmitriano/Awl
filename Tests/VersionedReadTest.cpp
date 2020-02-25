#include "Awl/Io/Context.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/IntRange.h"

#include <iostream>
#include <algorithm>
#include <functional>

using namespace awl::testing;

namespace
{
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
        bool y;

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
        std::vector<int> z{ 1, 2, 3 };
        int x;
        std::string w = "xyz";

        AWL_STRINGIZABLE(x, z, w)
    };

    AWL_MEMBERWISE_EQUATABLE(B2)

    struct C2
    {
        int x = 7;

        AWL_STRINGIZABLE(x)
    };

    AWL_MEMBERWISE_EQUATABLE(C2)

    static const A1 a1_expected = { 1, 2.0, "abc" };
    static const B1 b1_expected = { 1, true };

    static const A2 a2_expected = { a1_expected.b, 5, "xyz", a1_expected.c };
    static const B2 b2_expected = { std::vector<int>{ 1, 2, 3 },  b1_expected.x, "xyz" };
    static const C2 c2_expected = { 7 };

    using FieldV1 = std::variant<bool, char, int, float, double, std::string>;
    using FieldV2 = std::variant<bool, char, int, float, double, std::string, std::vector<int>>;

    using OldContext = awl::io::Context<std::variant<A1, B1>, FieldV1>;
    using NewContext = awl::io::Context<std::variant<A2, B2, C2>, FieldV2>;
}

namespace awl::io
{
    template<class Stream, class Struct, class Context>
    inline void ReadV(Stream & s, Struct & val, const Context & ctx)
    {
        if (ctx.serializeStructIndex)
        {
            typename Context::StructIndexType index;
            Read(s, index);
            constexpr size_t expected_index = Context::template StructIndex<Struct>;
            if (index != expected_index)
            {
                throw TypeMismatchException(typeid(Struct).name(), index, expected_index);
            }
        }

        auto & new_proto = ctx.template FindNewPrototype<Struct>();
        auto & old_proto = ctx.template FindOldPrototype<Struct>();
        
        auto & readers = ctx.template FindFieldReaders<Struct>();
        auto & skippers = ctx.GetFieldSkippers();

        const std::vector<size_t> & name_map = ctx.template FindProtoMap<Struct>();

        assert(name_map.size() == old_proto.GetCount());

        for (size_t old_index = 0; old_index < name_map.size(); ++old_index)
        {
            const auto old_field = old_proto.GetField(old_index);

            const size_t new_index = name_map[old_index];

            if (new_index == Prototype::NoIndex)
            {
                if (!ctx.allowDelete)
                {
                    throw FieldNotFoundException(old_field.name);
                }

                //Skip by type.
                skippers[old_field.type]->SkipField(s);
            }
            else
            {
                const auto new_field = new_proto.GetField(new_index);

                if (new_field.type != old_field.type)
                {
                    throw TypeMismatchException(new_field.name, new_field.type, old_field.type);
                }

                //But read by index.
                readers[new_index]->ReadField(s, val);
            }
        }
    }

    template<class Stream, class Struct, class Context>
    inline void WriteV(Stream & s, const Struct & val, const Context & ctx)
    {
        if (ctx.serializeStructIndex)
        {
            const typename Context::StructIndexType index = static_cast<typename Context::StructIndexType>(Context::template StructIndex<Struct>);
            Write(s, index);
        }

        Write(s, val);
    }
}

namespace
{
    void WriteDataV1(awl::io::SequentialOutputStream & out, size_t count, bool with_metadata)
    {
        OldContext ctx;
        ctx.Initialize();

        {
            auto & a1_proto = ctx.FindNewPrototype<A1>();
            Assert::IsTrue(a1_proto.GetCount() == 3);

            auto & b1_proto = ctx.FindNewPrototype<B1>();
            Assert::IsTrue(b1_proto.GetCount() == 2);
        }

        if (with_metadata)
        {
            ctx.WriteNewPrototypes(out);
        }

        for (size_t i : awl::make_count(count))
        {
            static_cast<void>(i);
            
            awl::io::WriteV(out, a1_expected, ctx);
            awl::io::WriteV(out, b1_expected, ctx);
        }
    }

    void ReadDataV1(awl::io::SequentialInputStream & in, size_t count)
    {
        OldContext ctx;
        ctx.ReadOldPrototypes(in);

        for (size_t i : awl::make_count(count))
        {
            static_cast<void>(i);

            A1 a1;
            B1 b1;

            awl::io::ReadV(in, a1, ctx);
            awl::io::ReadV(in, b1, ctx);

            Assert::IsTrue(std::make_tuple(a1, b1) == std::make_tuple(a1_expected, b1_expected));
        }

        Assert::IsTrue(in.End());
    }

    void ReadDataV2(awl::io::SequentialInputStream & in, size_t count)
    {
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

        for (size_t i : awl::make_count(count))
        {
            static_cast<void>(i);

            A2 a2;
            B2 b2;
            C2 c2;

            awl::io::ReadV(in, a2, ctx);

            //Version 1 data has B2 so the condition is true.
            Assert::IsTrue(ctx.HasOldPrototype<B2>());
            awl::io::ReadV(in, b2, ctx);

            //There is no C2 in version 1 so the condition is false.
            Assert::IsFalse(ctx.HasOldPrototype<C2>());

            //An example of how to read data that may not exist in a previous version.
            if (ctx.HasOldPrototype<C2>())
            {
                awl::io::ReadV(in, c2, ctx);
            }

            Assert::IsTrue(std::make_tuple(a2, b2, c2) == std::make_tuple(a2_expected, b2_expected, c2_expected));
        }

        Assert::IsTrue(in.End());
    }
}

AWT_TEST(VersionedRead)
{
    AWL_ATTRIBUTE(size_t, count, 1);

    size_t meta_size;
    size_t block_size;
    
    std::vector<uint8_t> v;

    //measure data size
    {
        awl::io::VectorOutputStream out(v);

        WriteDataV1(out, 0, true);

        meta_size = v.size();

        v.clear();
    }

    {
        awl::io::VectorOutputStream out(v);

        WriteDataV1(out, 1, false);

        block_size = v.size();

        v.clear();
    }

    const size_t mem_size = meta_size + block_size * count;

    context.out << _T("Meta size: ") << meta_size << _T(", block size: ") << block_size << _T(", allocating ") << mem_size << _T(" bytes of memory.") << std::endl;

    v.reserve(mem_size);

    context.out << _T("Memory has been allocated. ") << _T(", vector capacity: ") << v.capacity() << std::endl;

    {
        awl::io::VectorOutputStream out(v);

        WriteDataV1(out, count, true);
    }

    Assert::AreEqual(mem_size, v.size());
    
    context.out << _T("Test data has been written. ") << _T("Vector size: ") << v.size() << _T(", vector capacity: ") << v.capacity() << std::endl;

    {
        awl::io::VectorInputStream in(v);

        ReadDataV1(in, count);
    }

    context.out << _T("Version 1 has been read") << std::endl;

    {
        awl::io::VectorInputStream in(v);

        ReadDataV2(in, count);
    }

    context.out << _T("Version 2 has been read") << std::endl;
}
