#include "Awl/Io/Context.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Io/MeasureStream.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StopWatch.h"
#include "Awl/IntRange.h"

#include <iostream>
#include <algorithm>
#include <functional>
#include <chrono>
#include <memory>

#include "BenchmarkHelpers.h"

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

    class TestMeasureStream : public awl::io::SequentialOutputStream
    {
    public:

        void Write(const uint8_t * buffer, size_t count) override
        {
            static_cast<void>(buffer);
            m_pos += count;
        }

        size_t GetLength() const
        {
            return m_pos;
        }

    private:

        //prevent the optimization
        volatile size_t m_pos = 0;
    };
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
    template <class OutputStream>
    std::chrono::steady_clock::duration WriteDataV1(const awl::testing::TestContext & context, OutputStream & out, size_t count, bool with_metadata)
    {
        OldContext ctx;
        ctx.Initialize();

        {
            auto & a1_proto = ctx.FindNewPrototype<A1>();
            AWT_ASSERT_TRUE(a1_proto.GetCount() == 3);

            auto & b1_proto = ctx.FindNewPrototype<B1>();
            AWT_ASSERT_TRUE(b1_proto.GetCount() == 2);
        }

        if (with_metadata)
        {
            ctx.WriteNewPrototypes(out);
        }

        awl::StopWatch w;

        for (size_t i : awl::make_count(count))
        {
            static_cast<void>(i);
            
            awl::io::WriteV(out, a1_expected, ctx);
            awl::io::WriteV(out, b1_expected, ctx);
        }

        return w;
    }

    inline void SkipStructureIndex(awl::io::SequentialInputStream & in, OldContext & ctx)
    {
        if (ctx.serializeStructIndex)
        {
            typename OldContext::StructIndexType index;
            awl::io::Read(in, index);
        }
    }

    std::chrono::steady_clock::duration ReadDataNoV(const awl::testing::TestContext & context, awl::io::SequentialInputStream & in, size_t count)
    {
        //Skip metadata
        OldContext ctx;
        ctx.ReadOldPrototypes(in);

        awl::StopWatch w;

        for (size_t i : awl::make_count(count))
        {
            static_cast<void>(i);

            A1 a1;
            B1 b1;

            SkipStructureIndex(in, ctx);
            awl::io::Read(in, a1);

            SkipStructureIndex(in, ctx);
            awl::io::Read(in, b1);

            AWT_ASSERT_TRUE(std::make_tuple(a1, b1) == std::make_tuple(a1_expected, b1_expected));
        }

        AWT_ASSERT_TRUE(in.End());

        return w;
    }

    std::chrono::steady_clock::duration ReadDataV1(const awl::testing::TestContext & context, awl::io::SequentialInputStream & in, size_t count)
    {
        OldContext ctx;
        ctx.ReadOldPrototypes(in);

        awl::StopWatch w;
        
        for (size_t i : awl::make_count(count))
        {
            static_cast<void>(i);

            A1 a1;
            B1 b1;

            awl::io::ReadV(in, a1, ctx);
            awl::io::ReadV(in, b1, ctx);

            AWT_ASSERT_TRUE(std::make_tuple(a1, b1) == std::make_tuple(a1_expected, b1_expected));
        }

        AWT_ASSERT_TRUE(in.End());

        return w;
    }

    std::chrono::steady_clock::duration ReadDataV2(const awl::testing::TestContext & context, awl::io::SequentialInputStream & in, size_t count)
    {
        NewContext ctx;
        ctx.ReadOldPrototypes(in);

        {
            auto & a2_proto = ctx.FindNewPrototype<A2>();
            AWT_ASSERT_TRUE(a2_proto.GetCount() == 4);

            auto & b2_proto = ctx.FindNewPrototype<B2>();
            AWT_ASSERT_TRUE(b2_proto.GetCount() == 3);

            auto & a1_proto = ctx.FindOldPrototype<A2>();
            AWT_ASSERT_TRUE(a1_proto.GetCount() == 3);

            auto & b1_proto = ctx.FindOldPrototype<B2>();
            AWT_ASSERT_TRUE(b1_proto.GetCount() == 2);
        }

        awl::StopWatch w;
        
        for (size_t i : awl::make_count(count))
        {
            static_cast<void>(i);

            A2 a2;
            B2 b2;
            C2 c2;

            awl::io::ReadV(in, a2, ctx);

            //Version 1 data has B2 so the condition is true.
            AWT_ASSERT_TRUE(ctx.HasOldPrototype<B2>());
            awl::io::ReadV(in, b2, ctx);

            //There is no C2 in version 1 so the condition is false.
            AWT_ASSERT_FALSE(ctx.HasOldPrototype<C2>());

            //An example of how to read data that may not exist in a previous version.
            if (ctx.HasOldPrototype<C2>())
            {
                awl::io::ReadV(in, c2, ctx);
            }

            AWT_ASSERT_TRUE(std::make_tuple(a2, b2, c2) == std::make_tuple(a2_expected, b2_expected, c2_expected));
        }

        AWT_ASSERT_TRUE(in.End());

        return w;
    }

    size_t MeasureStreamSize(const awl::testing::TestContext & context, size_t count)
    {
        size_t meta_size;

        {
            awl::io::MeasureStream out;

            WriteDataV1(context, out, 0, true);

            meta_size = out.GetLength();
        }

        size_t block_size;

        {
            awl::io::MeasureStream out;

            WriteDataV1(context, out, 1, false);

            block_size = out.GetLength();
        }

        const size_t mem_size = meta_size + block_size * count;

        context.out << _T("Meta size: ") << meta_size << _T(", block size: ") << block_size << _T(", allocating ") << mem_size << _T(" bytes of memory.") << std::endl;

        return mem_size;
    }
}

AWT_TEST(VtsReadWrite)
{
    AWT_ATTRIBUTE(size_t, count, 1);
    AWT_FLAG(only_write);

    const size_t mem_size = MeasureStreamSize(context, count);

    std::vector<uint8_t> v;
    v.reserve(mem_size);

    context.out << v.capacity() << _T(" bytes of memory has been allocated. ") << std::endl;

    //do the test

    {
        awl::io::VectorOutputStream out(v);

        auto d = WriteDataV1(context, out, count, true);

        context.out << _T("Test data has been written. ");
        
        ReportCountAndSpeed(context, d, count, v.size());

        context.out << std::endl;
    }

    AWT_ASSERT_EQUAL(mem_size, v.size());
    AWT_ASSERT_EQUAL(mem_size, v.capacity());

    if (!only_write)
    {
        {
            awl::io::VectorInputStream in(v);

            auto d = ReadDataNoV(context, in, count);

            context.out << _T("Plain has been read. ");

            ReportCountAndSpeed(context, d, count, v.size());

            context.out << std::endl;
        }

        {
            awl::io::VectorInputStream in(v);

            auto d = ReadDataV1(context, in, count);

            context.out << _T("Version 1 has been read. ");

            ReportCountAndSpeed(context, d, count, v.size());

            context.out << std::endl;
        }

        {
            awl::io::VectorInputStream in(v);

            auto d = ReadDataV2(context, in, count);

            context.out << _T("Version 2 has been read. ");

            ReportCountAndSpeed(context, d, count, v.size());

            context.out << std::endl;
        }
    }
}

AWT_BENCHMARK(VtsMeasureInline)
{
    AWT_ATTRIBUTE(size_t, count, 1);

    TestMeasureStream out;

    auto d = WriteDataV1(context, out, count, true);

    context.out << _T("Test data has been written. ");

    ReportCountAndSpeed(context, d, count, out.GetLength());

    context.out << std::endl;
}

AWT_BENCHMARK(VtsMeasureVirtual)
{
    AWT_ATTRIBUTE(size_t, count, 1);

    TestMeasureStream out;

    auto d = WriteDataV1(context, static_cast<awl::io::SequentialOutputStream &>(out), count, true);

    context.out << _T("Test data has been written. ");

    ReportCountAndSpeed(context, d, count, out.GetLength());

    context.out << std::endl;
}

namespace
{
#pragma pack (push)
#pragma pack (1)

    struct APack1
    {
        int a;
        double b;
        //std::string c;
        size_t c_len;
        char c_data[3];
    };

    struct BPack1
    {
        int x;
        bool y;
    };

#pragma pack (pop)

    static_assert(sizeof(APack1) == sizeof(int) + sizeof(double) + sizeof(size_t) + 3);

    static const APack1 a_pack1_expected = { 1, 2.0, 3u, { 'a', 'b', 'c' } };
    static const BPack1 b_pack1_expected = { 1, true };

    template <class OutputStream, class T>
    inline void PlainWrite(OutputStream & out, const T & val)
    {
        const uint8_t * bytes = reinterpret_cast<const uint8_t *>(&val);
        out.Write(bytes, sizeof(T));
    }
    
    template <class OutputStream>
    std::chrono::steady_clock::duration WriteDataPack1(const awl::testing::TestContext & context, OutputStream & out, size_t count)
    {
        static_cast<void>(context);
        
        awl::StopWatch w;

        for (size_t i : awl::make_count(count))
        {
            static_cast<void>(i);

            OldContext::StructIndexType fakeIndex = 25;
            PlainWrite(out, fakeIndex);
            PlainWrite(out, a_pack1_expected);
            PlainWrite(out, fakeIndex);
            PlainWrite(out, b_pack1_expected);
        }

        return w;
    }
}

AWT_BENCHMARK(VtsMeasurePack1Inline)
{
    AWT_ATTRIBUTE(size_t, count, 1);

    TestMeasureStream out;

    auto d = WriteDataPack1(context, out, count);

    context.out << _T("Test data has been written. ");

    ReportCountAndSpeed(context, d, count, out.GetLength());

    context.out << std::endl;
}

AWT_BENCHMARK(VtsMeasurePack1Virtual)
{
    AWT_ATTRIBUTE(size_t, count, 1);

    TestMeasureStream out;

    auto d = WriteDataPack1(context, static_cast<awl::io::SequentialOutputStream &>(out), count);

    context.out << _T("Test data has been written. ");

    ReportCountAndSpeed(context, d, count, out.GetLength());

    context.out << std::endl;
}

namespace
{
    template <class OutputStream>
    std::chrono::steady_clock::duration WriteDataPlain(const awl::testing::TestContext & context, OutputStream & out, size_t count)
    {
        static_cast<void>(context);

        awl::StopWatch w;

        for (size_t i : awl::make_count(count))
        {
            static_cast<void>(i);

            OldContext::StructIndexType fakeIndex = 25;
            PlainWrite(out, fakeIndex);
            PlainWrite(out, a1_expected.a);
            PlainWrite(out, a1_expected.b);
            PlainWrite(out, a1_expected.c.length());
            out.Write(reinterpret_cast<const uint8_t *>(a1_expected.c.data()), a1_expected.c.length());
            PlainWrite(out, fakeIndex);
            PlainWrite(out, b1_expected.x);
            PlainWrite(out, b1_expected.y);
        }

        return w;
    }
}

AWT_BENCHMARK(VtsMeasurePlainInline)
{
    AWT_ATTRIBUTE(size_t, count, 1);

    TestMeasureStream out;

    auto d = WriteDataPlain(context, out, count);

    context.out << _T("Test data has been written. ");

    ReportCountAndSpeed(context, d, count, out.GetLength());

    context.out << std::endl;
}

AWT_BENCHMARK(VtsMeasurePlainVirtual)
{
    AWT_ATTRIBUTE(size_t, count, 1);

    TestMeasureStream out;

    auto d = WriteDataPlain(context, static_cast<awl::io::SequentialOutputStream &>(out), count);

    context.out << _T("Test data has been written. ");

    ReportCountAndSpeed(context, d, count, out.GetLength());

    context.out << std::endl;
}

AWT_BENCHMARK(VtsMemSetMove)
{
    AWT_ATTRIBUTE(size_t, count, 1);

    std::unique_ptr<uint8_t> p(new uint8_t[count]);

    {
        context.out << _T("std::memset: ");

        awl::StopWatch w;

        std::memset(p.get(), 25u, count);

        ReportSpeed(context, w, count);

        context.out << std::endl;
    }

    std::unique_ptr<uint8_t> p1(new uint8_t[count]);

    {
        context.out << _T("std::memmove: ");

        awl::StopWatch w;

        std::memmove(p1.get(), p.get(), count);

        ReportSpeed(context, w, count);

        context.out << std::endl;
    }
}

namespace
{
    class TestMemoryOutputStream : public awl::io::SequentialOutputStream
    {
    public:

        TestMemoryOutputStream(size_t size) : m_size(size), pBuf(new uint8_t[size]), m_p(pBuf.get())
        {
            std::memset(pBuf.get(), 0u, m_size);
        }

        void Write(const uint8_t * buffer, size_t count) override
        {
            std::memmove(m_p, buffer, count);
            m_p += count;
        }

        size_t GetCapacity() const
        {
            return m_size;
        }

        size_t GetLength() const
        {
            return m_p - pBuf.get();
        }

        void Reset()
        {
            m_p = pBuf.get();
        }

    private:

        const size_t m_size;
        std::unique_ptr<uint8_t> pBuf;
        uint8_t * m_p;
    };
}

AWT_TEST(VtsWriteMemoryStream)
{
    AWT_ATTRIBUTE(size_t, count, 1);

    const size_t mem_size = MeasureStreamSize(context, count);

    TestMemoryOutputStream out(mem_size);

    {
        auto d = WriteDataV1(context, out, count, true);

        context.out << _T("Test data has been written. ");

        AWT_ASSERT_EQUAL(mem_size, out.GetCapacity());
        AWT_ASSERT_EQUAL(mem_size, out.GetLength());

        ReportCountAndSpeed(context, d, count, mem_size);

        context.out << std::endl;
    }

    out.Reset();

    {
        awl::StopWatch w;

        for (size_t i : awl::make_count(mem_size))
        {
            const uint8_t val = static_cast<uint8_t>(i);
            out.Write(&val, 1);
        }

        context.out << _T("Bytes has been written. ");

        ReportSpeed(context, w, mem_size);

        context.out << std::endl;
    }
}

AWT_BENCHMARK(VtsVolatileInt)
{
    AWT_ATTRIBUTE(size_t, count, 1);

    volatile size_t val;

    const size_t write_count = count * (std::tuple_size_v<awl::tuplizable_traits<A1>::Tie> + std::tuple_size_v<awl::tuplizable_traits<B1>::Tie>);
        
    awl::StopWatch w;

    for (size_t i : awl::make_count(write_count))
    {
        val = i;
    }

    ReportCountAndSpeed(context, w, count, count * sizeof(val));

    context.out << std::endl;
}
