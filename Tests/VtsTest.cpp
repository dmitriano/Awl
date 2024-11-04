/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/Vts.h"
#include "Awl/Io/PlainReader.h"
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
#include <type_traits>
#include <cassert>
#include <vector>
#include <set>

#include "Helpers/BenchmarkHelpers.h"
#include "Helpers/FormattingHelpers.h"
#include "Experimental/Io/TrivialMemoryStream.h"
#include "Experimental/Io/SampleStreams.h"
#include "Experimental/TrivialAllocator.h"
#include "VtsData.h"

using namespace awl::testing;
using namespace awl::testing::helpers;

namespace
{
    template <class IStream>
    using OldReader = awl::io::Reader<V1, IStream>;

    template <class OStream>
    using OldWriter = awl::io::Writer<V1, OStream>;

    template <class IStream>
    using NewReader = awl::io::Reader<V2, IStream>;

    using OldVirtualReader = OldReader<awl::io::SequentialInputStream>;
    using OldVirtualWriter = OldWriter<awl::io::SequentialOutputStream>;
    using NewVirtualReader = NewReader<awl::io::SequentialInputStream>;

    static_assert(awl::io::vts_read_context<OldVirtualReader, awl::io::SequentialInputStream, v1::B>);
    static_assert(awl::io::vts_write_context<OldVirtualWriter, awl::io::SequentialOutputStream, v1::B>);
    static_assert(awl::io::vts_read_context<NewVirtualReader, awl::io::SequentialInputStream, v2::B>);

    using OldVectorReader = OldReader<awl::io::VectorInputStream>;
    using OldVectorWriter = OldWriter<awl::io::VectorOutputStream>;
    using NewVectorReader = NewReader<awl::io::VectorInputStream>;

    static_assert(awl::io::vts_read_context<OldVectorReader, awl::io::VectorInputStream, v1::B>);
    static_assert(awl::io::vts_write_context<OldVectorWriter, awl::io::VectorOutputStream, v1::B>);
    static_assert(awl::io::vts_read_context<NewVectorReader, awl::io::VectorInputStream, v2::B>);

    using OldTrivialReader = OldReader<awl::io::TrivialMemoryStream>;
    using OldTrivialWriter = OldWriter<awl::io::TrivialMemoryStream>;
    using NewTrivialReader = NewReader<awl::io::TrivialMemoryStream>;

    static_assert(awl::io::vts_read_context<OldTrivialReader, awl::io::TrivialMemoryStream, v1::B>);
    static_assert(awl::io::vts_write_context<OldTrivialWriter, awl::io::TrivialMemoryStream, v1::B>);
    static_assert(awl::io::vts_read_context<NewTrivialReader, awl::io::TrivialMemoryStream, v2::B>);

    constexpr size_t defaultElementCount = 1000;
}

namespace
{
    using Duration = std::chrono::steady_clock::duration;
    
    template <class Writer>
    Duration WriteDataV1(typename Writer::OutputStream & out, size_t element_count, bool with_metadata)
    {
        Writer ctx;

        {
            auto & a1_proto = ctx.template FindNewPrototype<v1::A>();
            AWT_ASSERT(a1_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<v1::A>::Tie>);

            auto & b1_proto = ctx.template FindNewPrototype<v1::B>();
            AWT_ASSERT(b1_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<v1::B>::Tie>);
        }

        if (with_metadata)
        {
            ctx.WriteNewPrototypes(out);
        }

        awl::StopWatch w;

        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            ctx.WriteV(out, v1::a_expected);
            ctx.WriteV(out, v1::b_expected);
        }

        return w;
    }

    template <class Reader>
    Duration ReadDataPlain(typename Reader::InputStream & in, size_t element_count)
    {
        {
            //Skip metadata
            Reader ctx;
            ctx.ReadOldPrototypes(in);
        }

        awl::io::PlainReader<typename Reader::Variant, typename Reader::InputStream> ctx;

        awl::StopWatch w;

        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            v1::A a1;
            ctx.ReadV(in, a1);
            AWT_ASSERT(a1 == v1::a_expected);

            v1::B b1;
            ctx.ReadV(in, b1);
            AWT_ASSERT(b1 == v1::b_expected);
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

            v1::A a1;
            ctx.ReadV(in, a1);
            AWT_ASSERT(a1 == v1::a_expected);

            v1::B b1;
            ctx.ReadV(in, b1);
            AWT_ASSERT(b1 == v1::b_expected);
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
            auto & a2_proto = ctx.template FindNewPrototype<v2::A>();
            AWT_ASSERT(a2_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<v2::A>::Tie>);

            auto & b2_proto = ctx.template FindNewPrototype<v2::B>();
            AWT_ASSERT(b2_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<v2::B>::Tie>);

            //auto & a1_proto = ctx.template FindOldPrototype<v2::A>();
            //AWT_ASSERT(a1_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<v1::A>::Tie>);

            //auto & b1_proto = ctx.template FindOldPrototype<v2::B>();
            //AWT_ASSERT(b1_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<v1::B>::Tie>);
        }

        awl::StopWatch w;
        
        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            v2::A a2;
            ctx.ReadV(in, a2);
            AWT_ASSERT(a2 == v2::a_expected);

            //Version 1 data has v2::B so the condition is true.
            v2::B b2;
            ctx.ReadV(in, b2);
            AWT_ASSERT(b2 == v2::b_expected);

            //An example of how to read data that may not exist in a previous version.
            if (false)
            {
                v2::C c2;
                ctx.ReadV(in, c2);
                AWT_ASSERT(c2 == v2::c_expected);
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
    AWT_ATTRIBUTE(size_t, write_count, 1);
    AWT_ATTRIBUTE(size_t, read_count, 1);

    AWT_ASSERT(write_count >= 1);

    const size_t mem_size = MeasureStreamSize(context, element_count);

    std::vector<uint8_t> v;
    v.reserve(mem_size);

    context.out << v.capacity() << _T(" bytes of memory has been allocated. ") << std::endl;

    {
        awl::io::VectorOutputStream out(v);

        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();
        
        for (auto i : awl::make_count(write_count))
        {
            static_cast<void>(i);

            v.resize(0);

            total_d += WriteDataV1<OldVectorWriter>(out, element_count, true);
        }

        context.out << _T("Test data has been written. ");
        
        helpers::ReportCountAndSpeed(context, total_d, element_count * write_count, v.size() * write_count);

        context.out << std::endl;
    }

    AWT_ASSERT_EQUAL(mem_size, v.size());
    AWT_ASSERT_EQUAL(mem_size, v.capacity());

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);

            awl::io::VectorInputStream in(v);

            total_d += ReadDataPlain<OldVectorReader>(in, element_count);
        }

        context.out << _T("Plain data has been read. ");

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, v.size() * read_count);

        context.out << std::endl;
    }

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);

            awl::io::VectorInputStream in(v);

            total_d += ReadDataV1<OldVectorReader>(in, element_count);
        }

        context.out << _T("Version 1 has been read. ");

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, v.size() * read_count);

        context.out << std::endl;
    }

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);

            awl::io::VectorInputStream in(v);

            total_d += ReadDataV2<NewVectorReader>(in, element_count);
        }

        context.out << _T("Version 2 has been read. ");

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, v.size() * read_count);

        context.out << std::endl;
    }
}

//store to/load of misaligned address
AWT_UNSTABLE_TEST(VtsReadWriteTrivialMemoryStream)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);
    AWT_ATTRIBUTE(size_t, write_count, 1);
    AWT_ATTRIBUTE(size_t, read_count, 1);

    AWT_ASSERT(write_count >= 1);

    const size_t mem_size = MeasureStreamSize(context, element_count);

    context.out << _T("Allocating ") << mem_size << _T(" bytes of memory.") << std::endl;

    //do the test

    awl::io::TrivialMemoryStream in(mem_size);
    awl::io::TrivialMemoryStream & out = in;

    AWT_ASSERT_EQUAL(mem_size, out.GetCapacity());

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(write_count))
        {
            static_cast<void>(i);

            total_d += WriteDataV1<OldTrivialWriter>(out, element_count, true);

            AWT_ASSERT_EQUAL(mem_size, out.GetLength());
            AWT_ASSERT(in.End());

            out.Reset();
        }

        context.out << _T("Test data has been written. ");

        helpers::ReportCountAndSpeed(context, total_d, element_count * write_count, mem_size * write_count);

        context.out << std::endl;
    }

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);

            total_d += ReadDataPlain<OldTrivialReader>(in, element_count);

            in.Reset();
        }

        context.out << _T("Plain data has been read. ");

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, mem_size * read_count);

        context.out << std::endl;
    }

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);

            total_d += ReadDataV1<OldTrivialReader>(in, element_count);

            in.Reset();
        }

        context.out << _T("Version 1 has been read. ");

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, mem_size * read_count);

        context.out << std::endl;
    }

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);

            total_d += ReadDataV2<NewTrivialReader>(in, element_count);

            in.Reset();
        }

        context.out << _T("Version 2 has been read. ");

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, mem_size * read_count);

        context.out << std::endl;
    }
}

AWT_BENCHMARK(VtsMeasureSerializationInlinedVirtual)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    using OldMeasureWriter = OldWriter<awl::io::VirtualMeasureStream>;

    awl::io::VirtualMeasureStream out;

    auto d = WriteDataV1<OldMeasureWriter>(out, element_count, true);

    context.out << _T("Test data has been written. ");

    helpers::ReportCountAndSpeed(context, d, element_count, out.GetLength());

    context.out << std::endl;

    AWT_ASSERT_EQUAL((MeasureStreamSize(context, element_count, true)), out.GetLength());
}

AWT_BENCHMARK(VtsMeasureSerializationVirtual)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    auto p_out = awl::io::CreateMeasureStream();

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

    auto p_out = awl::io::CreateFakeStream();

    auto d = WriteDataV1<OldVirtualWriter>(*p_out, element_count, true);

    context.out << _T("Test data has been written. ");

    helpers::ReportCountAndSpeed(context, d, element_count, mem_size);

    context.out << std::endl;
}

AWT_BENCHMARK(VtsMemSetMove)
{
    AWT_ATTRIBUTE(size_t, element_count, defaultElementCount);

    std::unique_ptr<uint8_t[]> p(new uint8_t[element_count]);

    {
        context.out << _T("std::memset: ");

        awl::StopWatch w;

        std::memset(p.get(), 25u, element_count);

        helpers::ReportSpeed(context, w, element_count);

        context.out << std::endl;
    }

    std::unique_ptr<uint8_t[]> p1(new uint8_t[element_count]);

    {
        context.out << _T("std::memmove: ");

        awl::StopWatch w;

        std::memmove(p1.get(), p.get(), element_count);

        helpers::ReportSpeed(context, w, element_count);

        context.out << std::endl;
    }

    {
        context.out << _T("vector<uint8_t>::insert: ");

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

//store to/load of misaligned address
AWT_UNSTABLE_TEST(VtsWriteMemoryStreamSwitch)
{
    TestWrite<awl::io::SwitchMemoryOutputStream>(context);
}

//store to/load of misaligned address
AWT_UNSTABLE_TEST(VtsWriteMemoryStreamConstexpr)
{
    TestWrite<awl::io::TrivialMemoryStream>(context);
}
