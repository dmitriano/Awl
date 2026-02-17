/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/Vts.h"
#include "Awl/Io/PlainReader.h"
#include "Awl/Io/MeasureStream.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StopWatch.h"
#include "Awl/IntRange.h"
#include "Awl/StringFormat.h"

#include <chrono>
#include <tuple>

#include "Tests/Helpers/BenchmarkHelpers.h"
#include "Tests/Helpers/FormattingHelpers.h"
#include "Tests/Experimental/Io/TrivialMemoryStream.h"
#include "Tests/Experimental/Io/SampleStreams.h"
#include "Tests/VtsData.h"

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

    using OldVirtualWriter = OldWriter<awl::io::SequentialOutputStream>;

    using OldTrivialReader = OldReader<awl::io::TrivialMemoryStream>;
    using OldTrivialWriter = OldWriter<awl::io::TrivialMemoryStream>;
    using NewTrivialReader = NewReader<awl::io::TrivialMemoryStream>;

    static_assert(awl::io::vts_write_context<OldVirtualWriter, awl::io::SequentialOutputStream, v1::B>);
    static_assert(awl::io::vts_read_context<OldTrivialReader, awl::io::TrivialMemoryStream, v1::B>);
    static_assert(awl::io::vts_write_context<OldTrivialWriter, awl::io::TrivialMemoryStream, v1::B>);
    static_assert(awl::io::vts_read_context<NewTrivialReader, awl::io::TrivialMemoryStream, v2::B>);

    constexpr size_t defaultElementCount = 1000;
}

namespace
{
    using Duration = std::chrono::steady_clock::duration;

    template <class Writer>
    Duration WriteDataV1(typename Writer::OutputStream& out, size_t element_count, bool with_metadata)
    {
        Writer ctx;

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
    Duration ReadDataPlain(typename Reader::InputStream& in, size_t element_count)
    {
        {
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
            AWL_ASSERT(a1 == v1::a_expected);

            v1::B b1;
            ctx.ReadV(in, b1);
            AWL_ASSERT(b1 == v1::b_expected);
        }

        AWL_ASSERT(in.End());
        return w;
    }

    template <class Reader>
    Duration ReadDataV1(typename Reader::InputStream& in, size_t element_count)
    {
        Reader ctx;
        ctx.ReadOldPrototypes(in);

        awl::StopWatch w;

        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            v1::A a1;
            ctx.ReadV(in, a1);
            AWL_ASSERT(a1 == v1::a_expected);

            v1::B b1;
            ctx.ReadV(in, b1);
            AWL_ASSERT(b1 == v1::b_expected);
        }

        AWL_ASSERT(in.End());
        return w;
    }

    template <class Reader>
    Duration ReadDataV2(typename Reader::InputStream& in, size_t element_count)
    {
        Reader ctx;
        ctx.ReadOldPrototypes(in);

        awl::StopWatch w;

        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            v2::A a2;
            ctx.ReadV(in, a2);
            AWL_ASSERT(a2 == v2::a_expected);

            v2::B b2;
            ctx.ReadV(in, b2);
            AWL_ASSERT(b2 == v2::b_expected);
        }

        AWL_ASSERT(in.End());
        return w;
    }

    size_t MeasureStreamSize(const TestContext& context, size_t element_count, bool include_meta = true)
    {
        using OldMeasureWriter = OldWriter<awl::io::MeasureStream>;

        size_t meta_size = 0;

        if (include_meta)
        {
            awl::io::MeasureStream out;
            WriteDataV1<OldMeasureWriter>(out, 0, true);
            meta_size = out.GetLength();
        }

        size_t block_size = 0;

        {
            awl::io::MeasureStream out;
            WriteDataV1<OldMeasureWriter>(out, 1, false);
            block_size = out.GetLength();
        }

        const size_t mem_size = meta_size + block_size * element_count;
        context.logger.debug(awl::format() << _T("block size: ") << block_size << _T(", allocating ") << mem_size << _T(" bytes of memory."));
        return mem_size;
    }

    template <class OutputStream>
    void TestWrite(const TestContext& context)
    {
        AWL_ATTRIBUTE(size_t, element_count, defaultElementCount);
        AWL_ATTRIBUTE(size_t, iteration_count, 1);

        using Writer = OldWriter<OutputStream>;

        const size_t mem_size = MeasureStreamSize(context, element_count, false);
        OutputStream out(mem_size);

        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(iteration_count))
        {
            static_cast<void>(i);

            total_d += WriteDataV1<Writer>(out, element_count, false);

            AWL_ASSERT_EQUAL(mem_size, out.GetCapacity());
            AWL_ASSERT_EQUAL(mem_size, out.GetLength());

            out.Reset();
        }

        helpers::ReportCountAndSpeed(context, total_d, element_count * iteration_count, mem_size * iteration_count);
        context.logger.debug(awl::format());
    }
}

AWL_TEST(VtsReadWriteTrivialMemoryStream)
{
    AWL_ATTRIBUTE(size_t, element_count, defaultElementCount);
    AWL_ATTRIBUTE(size_t, write_count, 1);
    AWL_ATTRIBUTE(size_t, read_count, 1);

    AWL_ASSERT(write_count >= 1);

    const size_t mem_size = MeasureStreamSize(context, element_count);

    awl::io::TrivialMemoryStream in(mem_size);
    awl::io::TrivialMemoryStream& out = in;

    AWL_ASSERT_EQUAL(mem_size, out.GetCapacity());

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(write_count))
        {
            static_cast<void>(i);

            total_d += WriteDataV1<OldTrivialWriter>(out, element_count, true);

            AWL_ASSERT_EQUAL(mem_size, out.GetLength());
            AWL_ASSERT(in.End());

            out.Reset();
        }

        helpers::ReportCountAndSpeed(context, total_d, element_count * write_count, mem_size * write_count);
    }

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);
            total_d += ReadDataPlain<OldTrivialReader>(in, element_count);
            in.Reset();
        }

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, mem_size * read_count);
    }

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);
            total_d += ReadDataV1<OldTrivialReader>(in, element_count);
            in.Reset();
        }

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, mem_size * read_count);
    }

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);
            total_d += ReadDataV2<NewTrivialReader>(in, element_count);
            in.Reset();
        }

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, mem_size * read_count);
    }
}

AWL_BENCHMARK(VtsMeasureSerializationInlinedVirtual)
{
    AWL_ATTRIBUTE(size_t, element_count, defaultElementCount);

    using OldMeasureWriter = OldWriter<awl::io::VirtualMeasureStream>;

    awl::io::VirtualMeasureStream out;

    auto d = WriteDataV1<OldMeasureWriter>(out, element_count, true);

    context.logger.debug(_T("Test data has been written. "));

    helpers::ReportCountAndSpeed(context, d, element_count, out.GetLength());

    context.logger.debug(awl::format());

    AWL_ASSERT_EQUAL((MeasureStreamSize(context, element_count, true)), out.GetLength());
}

AWL_BENCHMARK(VtsMeasureSerializationVirtual)
{
    AWL_ATTRIBUTE(size_t, element_count, defaultElementCount);

    auto p_out = awl::io::CreateMeasureStream();

    auto d = WriteDataV1<OldVirtualWriter>(*p_out, element_count, true);

    context.logger.debug(_T("Test data has been written. "));

    size_t len = (dynamic_cast<awl::io::MeasureStream&>(*p_out)).GetLength();

    helpers::ReportCountAndSpeed(context, d, element_count, len);

    context.logger.debug(awl::format());

    AWL_ASSERT_EQUAL((MeasureStreamSize(context, element_count, true)), len);
}

AWL_BENCHMARK(VtsMeasureSerializationFake)
{
    AWL_ATTRIBUTE(size_t, element_count, defaultElementCount);

    const size_t mem_size = MeasureStreamSize(context, element_count, false);

    auto p_out = awl::io::CreateFakeStream();

    auto d = WriteDataV1<OldVirtualWriter>(*p_out, element_count, true);

    context.logger.debug(_T("Test data has been written. "));

    helpers::ReportCountAndSpeed(context, d, element_count, mem_size);

    context.logger.debug(awl::format());
}

AWL_TEST(VtsWriteMemoryStreamMemmove)
{
    TestWrite<awl::io::VirtualMemoryOutputStream>(context);
}

//store to/load of misaligned address
AWL_UNSTABLE_TEST(VtsWriteMemoryStreamSwitch)
{
    TestWrite<awl::io::SwitchMemoryOutputStream>(context);
}

//store to/load of misaligned address
AWL_UNSTABLE_TEST(VtsWriteMemoryStreamConstexpr)
{
    TestWrite<awl::io::TrivialMemoryStream>(context);
}
