#include "Awl/Io/Context.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Io/MeasureStream.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StopWatch.h"
#include "Awl/IntRange.h"
#include "Awl/Crypto/Crc64.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <chrono>
#include <memory>
#include <type_traits>
#include <cassert>

#include "Helpers/BenchmarkHelpers.h"
#include "Helpers/FormattingHelpers.h"
#include "MemoryStream.h"

using namespace awl::testing;
using namespace awl::testing::helpers;

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

    using V1 = std::variant<A1, B1, bool, char, int, float, double, std::string>;
    using V2 = std::variant<A2, B2, bool, char, int, float, double, std::string, C2, std::vector<int>>;

    using OldContext = awl::io::Context<V1>;
    using NewContext = awl::io::Context<V2>;

    class VirtualMeasureStream : public awl::io::SequentialOutputStream
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

    class InlineMeasureStream
    {
    public:

        void Write(const uint8_t * buffer, size_t count)
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

    constexpr size_t defaultElementCount = 1000;
}

namespace VtsTest
{
    std::unique_ptr<awl::io::SequentialOutputStream> CreateFakeStream();

    std::unique_ptr<awl::io::SequentialOutputStream> CreateMeasureStream();
}

namespace
{
    template <class OutputStream>
    std::chrono::steady_clock::duration WriteDataV1(OutputStream & out, size_t element_count, bool with_metadata)
    {
        OldContext ctx;
        ctx.Initialize();

        {
            auto & a1_proto = ctx.FindNewPrototype<A1>();
            AWT_ASSERT(a1_proto.GetCount() == 3);

            auto & b1_proto = ctx.FindNewPrototype<B1>();
            AWT_ASSERT(b1_proto.GetCount() == 2);
        }

        if constexpr (std::is_base_of_v<awl::io::SequentialOutputStream, OutputStream>)
        {
            if (with_metadata)
            {
                ctx.WriteNewPrototypes(out);
            }
        }
        else
        {
            assert(!with_metadata);
            static_cast<void>(with_metadata);
        }

        awl::StopWatch w;

        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);
            
            ctx.WriteV(out, a1_expected);
            ctx.WriteV(out, b1_expected);
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

    std::chrono::steady_clock::duration ReadDataNoV(awl::io::SequentialInputStream & in, size_t element_count)
    {
        //Skip metadata
        OldContext ctx;
        ctx.ReadOldPrototypes(in);

        awl::StopWatch w;

        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            A1 a1;
            B1 b1;

            SkipStructureIndex(in, ctx);
            awl::io::Read(in, a1);

            SkipStructureIndex(in, ctx);
            awl::io::Read(in, b1);

            AWT_ASSERT(std::make_tuple(a1, b1) == std::make_tuple(a1_expected, b1_expected));
        }

        AWT_ASSERT(in.End());

        return w;
    }

    std::chrono::steady_clock::duration ReadDataV1(awl::io::SequentialInputStream & in, size_t element_count)
    {
        OldContext ctx;
        ctx.ReadOldPrototypes(in);

        awl::StopWatch w;
        
        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            A1 a1;
            B1 b1;

            ctx.ReadV(in, a1);
            ctx.ReadV(in, b1);

            AWT_ASSERT(std::make_tuple(a1, b1) == std::make_tuple(a1_expected, b1_expected));
        }

        AWT_ASSERT(in.End());

        return w;
    }

    std::chrono::steady_clock::duration ReadDataV2(awl::io::SequentialInputStream & in, size_t element_count)
    {
        NewContext ctx;
        ctx.ReadOldPrototypes(in);

        {
            auto & a2_proto = ctx.FindNewPrototype<A2>();
            AWT_ASSERT(a2_proto.GetCount() == 4);

            auto & b2_proto = ctx.FindNewPrototype<B2>();
            AWT_ASSERT(b2_proto.GetCount() == 3);

            auto & a1_proto = ctx.FindOldPrototype<A2>();
            AWT_ASSERT(a1_proto.GetCount() == 3);

            auto & b1_proto = ctx.FindOldPrototype<B2>();
            AWT_ASSERT(b1_proto.GetCount() == 2);
        }

        awl::StopWatch w;
        
        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            A2 a2;
            B2 b2;
            C2 c2;

            ctx.ReadV(in, a2);

            //Version 1 data has B2 so the condition is true.
            AWT_ASSERT(ctx.HasOldPrototype<B2>());
            ctx.ReadV(in, b2);

            //There is no C2 in version 1 so the condition is false.
            AWT_ASSERT_FALSE(ctx.HasOldPrototype<C2>());

            //An example of how to read data that may not exist in a previous version.
            if (ctx.HasOldPrototype<C2>())
            {
                ctx.ReadV(in, c2);
            }

            AWT_ASSERT(std::make_tuple(a2, b2, c2) == std::make_tuple(a2_expected, b2_expected, c2_expected));
        }

        AWT_ASSERT(in.End());

        return w;
    }

    size_t MeasureStreamSize(const TestContext & context, size_t element_count, bool include_meta = true)
    {
        size_t meta_size = 0;

        if (include_meta)
        {
            awl::io::MeasureStream out;

            WriteDataV1(out, 0, true);

            meta_size = out.GetLength();
        }

        size_t block_size;

        {
            awl::io::MeasureStream out;

            WriteDataV1(out, 1, false);

            block_size = out.GetLength();
        }

        const size_t mem_size = meta_size + block_size * element_count;

        if (include_meta)
        {
            context.out << _T("Meta size: ") << meta_size << _T(", ");
        }

        context.out << _T("block size: ") << block_size << _T(", allocating ") << mem_size << _T(" bytes of memory.") << std::endl;

        return mem_size;
    }
}

AWT_TEST(VtsReadWrite)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);
    AWT_ATTRIBUTE(size_t, iteration_count, 1);
    AWT_FLAG(only_write);

    const size_t mem_size = MeasureStreamSize(context, element_count);

    std::vector<uint8_t> v;
    v.reserve(mem_size);

    context.out << v.capacity() << _T(" bytes of memory has been allocated. ") << std::endl;

    //do the test

    {
        awl::io::VectorOutputStream out(v);

        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();
        
        for (auto i : awl::make_count(iteration_count))
        {
            static_cast<void>(i);

            v.resize(0);

            total_d += WriteDataV1(out, element_count, true);
        }

        context.out << _T("Test data has been written. ");
        
        helpers::ReportCountAndSpeed(context, total_d, element_count * iteration_count, v.size() * iteration_count);

        context.out << std::endl;
    }

    AWT_ASSERT_EQUAL(mem_size, v.size());
    AWT_ASSERT_EQUAL(mem_size, v.capacity());

    if (!only_write)
    {
        {
            awl::io::VectorInputStream in(v);

            auto d = ReadDataNoV(in, element_count);

            context.out << _T("Plain has been read. ");

            helpers::ReportCountAndSpeed(context, d, element_count, v.size());

            context.out << std::endl;
        }

        {
            awl::io::VectorInputStream in(v);

            auto d = ReadDataV1(in, element_count);

            context.out << _T("Version 1 has been read. ");

            helpers::ReportCountAndSpeed(context, d, element_count, v.size());

            context.out << std::endl;
        }

        {
            awl::io::VectorInputStream in(v);

            auto d = ReadDataV2(in, element_count);

            context.out << _T("Version 2 has been read. ");

            helpers::ReportCountAndSpeed(context, d, element_count, v.size());

            context.out << std::endl;
        }
    }
}

AWT_BENCHMARK(VtsMeasureSerializationInline)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    VirtualMeasureStream out;

    auto d = WriteDataV1(out, element_count, true);

    context.out << _T("Test data has been written. ");

    helpers::ReportCountAndSpeed(context, d, element_count, out.GetLength());

    context.out << std::endl;

    AWT_ASSERT_EQUAL((MeasureStreamSize(context, element_count, true)), out.GetLength());
}

AWT_BENCHMARK(VtsMeasureSerializationVirtual)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    auto p_out = VtsTest::CreateMeasureStream();

    auto d = WriteDataV1(*p_out, element_count, true);

    context.out << _T("Test data has been written. ");

    size_t len = (dynamic_cast<awl::io::MeasureStream &>(*p_out)).GetLength();

    helpers::ReportCountAndSpeed(context, d, element_count, len);

    context.out << std::endl;

    AWT_ASSERT_EQUAL((MeasureStreamSize(context, element_count, true)), len);
}

AWT_BENCHMARK(VtsMeasureSerializationFake)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    const size_t mem_size = MeasureStreamSize(context, element_count, false);

    auto p_out = VtsTest::CreateFakeStream();

    auto d = WriteDataV1(*p_out, element_count, true);

    context.out << _T("Test data has been written. ");

    helpers::ReportCountAndSpeed(context, d, element_count, mem_size);

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
    std::chrono::steady_clock::duration WriteDataPack1(OutputStream & out, size_t element_count)
    {
        awl::StopWatch w;

        for (size_t i : awl::make_count(element_count))
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
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    const size_t mem_size = MeasureStreamSize(context, element_count, false);

    VirtualMeasureStream out;

    auto d = WriteDataPack1(out, element_count);

    AWT_ASSERT(mem_size == out.GetLength());

    context.out << _T("Test data has been written. ");

    helpers::ReportCountAndSpeed(context, d, element_count, out.GetLength());

    context.out << std::endl;
}

AWT_BENCHMARK(VtsMeasurePack1Virtual)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    VirtualMeasureStream out;

    auto d = WriteDataPack1(static_cast<awl::io::SequentialOutputStream &>(out), element_count);

    context.out << _T("Test data has been written. ");

    helpers::ReportCountAndSpeed(context, d, element_count, out.GetLength());

    context.out << std::endl;
}

namespace
{
    template <class OutputStream>
    std::chrono::steady_clock::duration WriteDataPlain(OutputStream & out, size_t element_count)
    {
        awl::StopWatch w;

        for (size_t i : awl::make_count(element_count))
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
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    const size_t mem_size = MeasureStreamSize(context, element_count, false);
    
    InlineMeasureStream out;

    auto d = WriteDataPlain(out, element_count);

    AWT_ASSERT(mem_size == out.GetLength());

    context.out << _T("Test data has been written. ");

    helpers::ReportCountAndSpeed(context, d, element_count, out.GetLength());

    context.out << std::endl;
}

AWT_BENCHMARK(VtsMeasurePlainVirtual)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    auto p_out = VtsTest::CreateMeasureStream();

    auto d = WriteDataPlain(*p_out, element_count);

    size_t len = (dynamic_cast<awl::io::MeasureStream &>(*p_out)).GetLength();

    helpers::ReportCountAndSpeed(context, d, element_count, len);

    context.out << std::endl;

    AWT_ASSERT_EQUAL((MeasureStreamSize(context, element_count, false)), len);
}

AWT_BENCHMARK(VtsMeasurePlainFake)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    const size_t mem_size = MeasureStreamSize(context, element_count, false);

    auto p_out = VtsTest::CreateFakeStream();

    auto d = WriteDataPlain(*p_out, element_count);

    context.out << _T("Test data has been written. ");

    helpers::ReportCountAndSpeed(context, d, element_count, mem_size);

    context.out << std::endl;
}

AWT_BENCHMARK(VtsMemSetMove)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    std::unique_ptr<uint8_t> p(new uint8_t[element_count]);

    {
        context.out << _T("std::memset: ");

        awl::StopWatch w;

        std::memset(p.get(), 25u, element_count);

        helpers::ReportSpeed(context, w, element_count);

        context.out << std::endl;
    }

    std::unique_ptr<uint8_t> p1(new uint8_t[element_count]);

    {
        context.out << _T("std::memmove: ");

        awl::StopWatch w;

        std::memmove(p1.get(), p.get(), element_count);

        helpers::ReportSpeed(context, w, element_count);

        context.out << std::endl;
    }

    {
        context.out << _T("std::vector<uint8_t>::insert: ");

        std::vector<uint8_t> v;
        v.reserve(element_count);
        AWT_ASSERT_EQUAL(element_count, v.capacity());

        awl::StopWatch w;

        v.insert(v.end(), p.get(), p.get() + element_count);
        AWT_ASSERT_EQUAL(element_count, v.capacity());
        AWT_ASSERT_EQUAL(element_count, v.size());

        helpers::ReportSpeed(context, w, element_count);

        context.out << std::endl;
    }
}

namespace
{
    template <class T>
    constexpr inline void PlainCopy(uint8_t * p_dest, const uint8_t * p_src)
    {
        T * dest = reinterpret_cast<T *>(p_dest);
        const T * src = reinterpret_cast<const T *>(p_src);
        *dest = *src;
    }

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
            //std::copy(buffer, buffer + count, m_p);
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

        const uint8_t * begin() const { return pBuf.get(); }
        const uint8_t * end() const { return pBuf.get() + m_size; }

    private:

        const size_t m_size;
        std::unique_ptr<uint8_t> pBuf;
        uint8_t * m_p;
    };

    class SwitchMemoryOutputStream
    {
    public:

        //new uint8_t[size] is not constexpr and allocating 64K on the stack probably is not a good idea.
        SwitchMemoryOutputStream(size_t size) : m_size(size), pBuf(new uint8_t[size]), m_p(pBuf)
        {
            std::memset(pBuf, 0u, m_size);
        }

        ~SwitchMemoryOutputStream()
        {
            delete pBuf;
        }

        //To make this look better and get gid of switch operator we would probably define
        //the specialization of Read/Write functions not only for the type
        //but also for the stream.
        constexpr void Write(const uint8_t * buffer, size_t count)
        {
            switch (count)
            {
            case 1:
                PlainCopy<uint8_t>(m_p, buffer);
                break;
            case 2:
                PlainCopy<uint16_t>(m_p, buffer);
                break;
            case 4:
                PlainCopy<uint32_t>(m_p, buffer);
                break;
            case 8:
                PlainCopy<uint64_t>(m_p, buffer);
                break;
            default:
                //memcpy, memmove, and memset are obsolete!
                //std::copy is constexpr in C++ 20.
                //std::copy(buffer, buffer + count, m_p);
                awl::io::StdCopy(buffer, buffer + count, m_p);
                break;
            }

            m_p += count;
        }

        size_t GetCapacity() const
        {
            return m_size;
        }

        size_t GetLength() const
        {
            return m_p - pBuf;
        }

        void Reset()
        {
            m_p = pBuf;
        }

        const uint8_t * begin() const { return pBuf; }
        const uint8_t * end() const { return pBuf + m_size; }

    private:

        const size_t m_size;
        uint8_t * pBuf;
        uint8_t * m_p;
    };

}

namespace
{
    template <class OutputStream>
    void TestMemoryStream(const TestContext & context)
    {
        AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);
        AWT_ATTRIBUTE(size_t, iteration_count, 1);

        const size_t mem_size = MeasureStreamSize(context, element_count, false);

        OutputStream out(mem_size);

        {
            std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

            for (auto i : awl::make_count(iteration_count))
            {
                static_cast<void>(i);

                total_d += WriteDataV1(out, element_count, false);

                AWT_ASSERT_EQUAL(mem_size, out.GetCapacity());
                AWT_ASSERT_EQUAL(mem_size, out.GetLength());

                out.Reset();
            }

            awl::crypto::Crc64 hash;
            auto h = hash(out.begin(), out.end());
            context.out << _T("Test data has been written. Buffer hash=") << h << std::endl;

            helpers::ReportCountAndSpeed(context, total_d, element_count * iteration_count, mem_size * iteration_count);
            context.out << std::endl;
        }

        /*
        out.Reset();

        {
            awl::StopWatch w;

            for (size_t i : awl::make_count(mem_size))
            {
                const uint8_t val = static_cast<uint8_t>(i);
                out.Write(&val, 1);
            }

            context.out << _T("Bytes has been written. ");

            AWT_ASSERT_EQUAL(mem_size, out.GetCapacity());
            AWT_ASSERT_EQUAL(mem_size, out.GetLength());

            helpers::ReportSpeed(context, w, mem_size);

          context.out << std::endl;
        }
        */
    }
}

AWT_TEST(VtsWriteMemoryStreamMemmove)
{
    TestMemoryStream<TestMemoryOutputStream>(context);
}

AWT_TEST(VtsWriteMemoryStreamSwitch)
{
    TestMemoryStream<SwitchMemoryOutputStream>(context);
}

AWT_TEST(VtsWriteMemoryStreamConstexpr)
{
    TestMemoryStream<awl::io::MemoryOutputStream>(context);
}

AWT_BENCHMARK(VtsVolatileInt)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    volatile size_t val;

    constexpr size_t field_count = std::tuple_size_v<awl::tuplizable_traits<A1>::Tie> + std::tuple_size_v<awl::tuplizable_traits<B1>::Tie>;

    const size_t mem_size = MeasureStreamSize(context, 1, false);

    const size_t write_count = element_count * field_count;

    awl::StopWatch w;

    for (size_t i : awl::make_count(write_count))
    {
        val = i;
    }

    helpers::ReportCountAndSpeed(context, w, element_count, element_count * mem_size);
    context.out << std::endl;
    helpers::ReportCountAndSpeed(context, w, write_count, write_count * sizeof(val));
    context.out << std::endl;
}

AWT_BENCHMARK(VtsVolatileStream)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    InlineMeasureStream out;

    constexpr size_t field_count = std::tuple_size_v<awl::tuplizable_traits<A1>::Tie> + std::tuple_size_v<awl::tuplizable_traits<B1>::Tie>;

    const size_t mem_size = MeasureStreamSize(context, 1, false);

    const size_t write_count = element_count * field_count;

    awl::StopWatch w;

    for (size_t i : awl::make_count(write_count))
    {
        PlainWrite(out, i);
    }

    helpers::ReportCountAndSpeed(context, w, element_count, element_count * mem_size);
    context.out << std::endl;
    helpers::ReportCountAndSpeed(context, w, write_count, write_count * sizeof(size_t));
    context.out << std::endl;
}

AWT_BENCHMARK(VtsVolatileFake)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    auto p_out = VtsTest::CreateFakeStream();

    constexpr size_t field_count = std::tuple_size_v<awl::tuplizable_traits<A1>::Tie> + std::tuple_size_v<awl::tuplizable_traits<B1>::Tie>;

    const size_t mem_size = MeasureStreamSize(context, 1, false);

    const size_t write_count = element_count * field_count;

    awl::StopWatch w;

    for (size_t i : awl::make_count(write_count))
    {
        PlainWrite(*p_out, i);
    }

    helpers::ReportCountAndSpeed(context, w, element_count, element_count * mem_size);
    context.out << std::endl;
    helpers::ReportCountAndSpeed(context, w, write_count, write_count * sizeof(size_t));
    context.out << std::endl;
}
