#include "Awl/Io/Serializer.h"
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
        A1 a;
        A1 b;
        int x;
        bool y;

        AWL_STRINGIZABLE(a, b, x, y)
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
        A2 a;
        std::vector<int> z{ 1, 2, 3 };
        int x;
        std::string w = "xyz";

        AWL_STRINGIZABLE(a, x, z, w)
    };

    AWL_MEMBERWISE_EQUATABLE(B2)

    struct C2
    {
        int x = 7;

        AWL_STRINGIZABLE(x)
    };

    AWL_MEMBERWISE_EQUATABLE(C2)

    static const A1 a1_expected = { 1, 2.0, "abc" };
    static const B1 b1_expected = { a1_expected, a1_expected, 1, true };

    static const A2 a2_expected = { a1_expected.b, 5, "xyz", a1_expected.c };
    static const B2 b2_expected = { a2_expected, std::vector<int>{ 1, 2, 3 },  b1_expected.x, "xyz" };
    static const C2 c2_expected = { 7 };

    using V1 = std::variant<A1, B1, bool, char, int, float, double, std::string>;
    using V2 = std::variant<A2, B2, bool, char, int, float, double, std::string, C2, std::vector<int>>;

    template <class IStream>
    using OldReader = awl::io::Reader<V1, IStream>;

    template <class OStream>
    using OldWriter = awl::io::Writer<V1, OStream>;

    template <class IStream>
    using NewReader = awl::io::Reader<V2, IStream>;

    using OldVirtualReader = OldReader<awl::io::SequentialInputStream>;
    using OldVirtualWriter = OldWriter<awl::io::SequentialOutputStream>;
    using NewVirtualReader = NewReader<awl::io::SequentialInputStream>;

    using OldVectorReader = OldReader<awl::io::VectorInputStream>;
    using OldVectorWriter = OldWriter<awl::io::VectorOutputStream>;
    using NewVectorReader = NewReader<awl::io::VectorInputStream>;

    using OldTrivialReader = OldReader<awl::io::TrivialMemoryStream>;
    using OldTrivialWriter = OldWriter<awl::io::TrivialMemoryStream>;
    using NewTrivialReader = NewReader<awl::io::TrivialMemoryStream>;

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
    using Duration = std::chrono::steady_clock::duration;
    
    template <class Writer>
    Duration WriteDataV1(typename Writer::OutputStream & out, size_t element_count, bool with_metadata)
    {
        Writer ctx;

        {
            auto & a1_proto = ctx.template FindNewPrototype<A1>();
            AWT_ASSERT(a1_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<A1>::Tie>);

            auto & b1_proto = ctx.template FindNewPrototype<B1>();
            AWT_ASSERT(b1_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<B1>::Tie>);
        }

        if (with_metadata)
        {
            ctx.WriteNewPrototypes(out);
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

    template <class Reader>
    Duration ReadDataPlain(typename Reader::InputStream & in, size_t element_count)
    {
        //Skip metadata
        Reader ctx;
        ctx.ReadOldPrototypes(in);

        awl::StopWatch w;

        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            A1 a1;
            ctx.ReadPlain(in, a1);
            AWT_ASSERT(a1 == a1_expected);

            B1 b1;
            ctx.ReadPlain(in, b1);
            AWT_ASSERT(b1 == b1_expected);
        }

        AWT_ASSERT(in.End());

        return w;
    }

    template <class Reader>
    Duration ReadDataV1(typename Reader::InputStream & in, size_t element_count)
    {
        Reader ctx;
        ctx.ReadOldPrototypes(in);

        awl::StopWatch w;
        
        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            A1 a1;
            ctx.ReadV(in, a1);
            AWT_ASSERT(a1 == a1_expected);

            B1 b1;
            ctx.ReadV(in, b1);
            AWT_ASSERT(b1 == b1_expected);
        }

        AWT_ASSERT(in.End());

        return w;
    }

    template <class Reader>
    Duration ReadDataV2(typename Reader::InputStream & in, size_t element_count)
    {
        Reader ctx;
        ctx.ReadOldPrototypes(in);

        {
            auto & a2_proto = ctx.template FindNewPrototype<A2>();
            AWT_ASSERT(a2_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<A2>::Tie>);

            auto & b2_proto = ctx.template FindNewPrototype<B2>();
            AWT_ASSERT(b2_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<B2>::Tie>);

            auto & a1_proto = ctx.template FindOldPrototype<A2>();
            AWT_ASSERT(a1_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<A1>::Tie>);

            auto & b1_proto = ctx.template FindOldPrototype<B2>();
            AWT_ASSERT(b1_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<B1>::Tie>);
        }

        awl::StopWatch w;
        
        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            A2 a2;
            ctx.ReadV(in, a2);
            AWT_ASSERT(a2 == a2_expected);

            //Version 1 data has B2 so the condition is true.
            AWT_ASSERT(ctx.template HasOldPrototype<B2>());
            B2 b2;
            ctx.ReadV(in, b2);
            AWT_ASSERT(b2 == b2_expected);

            //There is no C2 in version 1 so the condition is false.
            AWT_ASSERT_FALSE(ctx.template HasOldPrototype<C2>());

            //An example of how to read data that may not exist in a previous version.
            if (ctx.template HasOldPrototype<C2>())
            {
                C2 c2;
                ctx.ReadV(in, c2);
                AWT_ASSERT(c2 == c2_expected);
            }
        }

        AWT_ASSERT(in.End());

        return w;
    }

    size_t MeasureStreamSize(const TestContext & context, size_t element_count, bool include_meta = true)
    {
        using OldMeasureWriter = OldWriter<awl::io::MeasureStream>;

        size_t meta_size = 0;

        if (include_meta)
        {
            awl::io::MeasureStream out;

            WriteDataV1<OldMeasureWriter>(out, 0, true);

            meta_size = out.GetLength();
        }

        size_t block_size;

        {
            awl::io::MeasureStream out;

            WriteDataV1<OldMeasureWriter>(out, 1, false);

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

    //template <class Reader>
    //inline void TestWriteDataV1(typename Reader::OutputStream & out, size_t element_count)
    //{
    //}
}

AWT_TEST(VtsReadWriteVectorStream)
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

            total_d += WriteDataV1<OldVectorWriter>(out, element_count, true);
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

            auto d = ReadDataPlain<OldVectorReader>(in, element_count);

            context.out << _T("Plain data has been read. ");

            helpers::ReportCountAndSpeed(context, d, element_count, v.size());

            context.out << std::endl;
        }

        {
            awl::io::VectorInputStream in(v);

            auto d = ReadDataV1<OldVectorReader>(in, element_count);

            context.out << _T("Version 1 has been read. ");

            helpers::ReportCountAndSpeed(context, d, element_count, v.size());

            context.out << std::endl;
        }

        {
            awl::io::VectorInputStream in(v);

            auto d = ReadDataV2<NewVectorReader>(in, element_count);

            context.out << _T("Version 2 has been read. ");

            helpers::ReportCountAndSpeed(context, d, element_count, v.size());

            context.out << std::endl;
        }
    }
}

AWT_TEST(VtsReadWriteTrivialMemoryStream)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);
    AWT_ATTRIBUTE(size_t, iteration_count, 1);
    AWT_FLAG(only_write);

    const size_t mem_size = MeasureStreamSize(context, element_count);

    context.out << _T("Allocating ") << mem_size << _T(" bytes of memory.") << std::endl;

    //do the test

    awl::io::TrivialMemoryStream in(mem_size);
    awl::io::TrivialMemoryStream & out = in;

    AWT_ASSERT_EQUAL(mem_size, out.GetCapacity());

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(iteration_count))
        {
            static_cast<void>(i);

            total_d += WriteDataV1<OldTrivialWriter>(out, element_count, true);

            AWT_ASSERT_EQUAL(mem_size, out.GetLength());
            AWT_ASSERT(in.End());

            out.Reset();
        }

        context.out << _T("Test data has been written. ");

        helpers::ReportCountAndSpeed(context, total_d, element_count * iteration_count, mem_size * iteration_count);

        context.out << std::endl;
    }

    if (!only_write)
    {
        {
            in.Reset();

            auto d = ReadDataPlain<OldTrivialReader>(in, element_count);

            context.out << _T("Plain data has been read. ");

            helpers::ReportCountAndSpeed(context, d, element_count, mem_size);

            context.out << std::endl;
        }

        {
            in.Reset();

            auto d = ReadDataV1<OldTrivialReader>(in, element_count);

            context.out << _T("Version 1 has been read. ");

            helpers::ReportCountAndSpeed(context, d, element_count, mem_size);

            context.out << std::endl;
        }

        {
            in.Reset();

            auto d = ReadDataV2<NewTrivialReader>(in, element_count);

            context.out << _T("Version 2 has been read. ");

            helpers::ReportCountAndSpeed(context, d, element_count, mem_size);

            context.out << std::endl;
        }
    }
}

AWT_BENCHMARK(VtsMeasureSerializationInline)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    using OldMeasureWriter = OldWriter<VirtualMeasureStream>;

    VirtualMeasureStream out;

    auto d = WriteDataV1<OldMeasureWriter>(out, element_count, true);

    context.out << _T("Test data has been written. ");

    helpers::ReportCountAndSpeed(context, d, element_count, out.GetLength());

    context.out << std::endl;

    AWT_ASSERT_EQUAL((MeasureStreamSize(context, element_count, true)), out.GetLength());
}

AWT_BENCHMARK(VtsMeasureSerializationVirtual)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    auto p_out = VtsTest::CreateMeasureStream();

    auto d = WriteDataV1<OldVirtualWriter>(*p_out, element_count, true);

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

    auto d = WriteDataV1<OldVirtualWriter>(*p_out, element_count, true);

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
    template <class OutputStream>
    void TestWrite(const TestContext & context)
    {
        AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);
        AWT_ATTRIBUTE(size_t, iteration_count, 1);

        using Writer = OldWriter<OutputStream>;

        const size_t mem_size = MeasureStreamSize(context, element_count, false);

        OutputStream out(mem_size);

        {
            std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

            for (auto i : awl::make_count(iteration_count))
            {
                static_cast<void>(i);

                total_d += WriteDataV1<Writer>(out, element_count, false);

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
    }
}

AWT_TEST(VtsWriteMemoryStreamMemmove)
{
    TestWrite<awl::io::VirtualMemoryOutputStream>(context);
}

AWT_TEST(VtsWriteMemoryStreamSwitch)
{
    TestWrite<awl::io::SwitchMemoryOutputStream>(context);
}

AWT_TEST(VtsWriteMemoryStreamConstexpr)
{
    TestWrite<awl::io::TrivialMemoryStream>(context);
}
