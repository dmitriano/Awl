/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"

#include <chrono>

#include "Tests/Helpers/BenchmarkHelpers.h"
#include "Tests/Helpers/FormattingHelpers.h"
#include "Tests/Experimental/Io/TrivialMemoryStream.h"
#include "Tests/Experimental/Io/SampleStreams.h"
#include "Tests/VtsTestCommon.h"

using namespace awl::testing;
using namespace awl::testing::helpers;

namespace
{
    using OldVirtualWriter = awl::testing::vts_common::OldWriter<awl::io::SequentialOutputStream>;

    using OldTrivialReader = awl::testing::vts_common::OldReader<awl::io::TrivialMemoryStream>;
    using OldTrivialWriter = awl::testing::vts_common::OldWriter<awl::io::TrivialMemoryStream>;
    using NewTrivialReader = awl::testing::vts_common::NewReader<awl::io::TrivialMemoryStream>;

    static_assert(awl::io::vts_write_context<OldVirtualWriter, awl::io::SequentialOutputStream, v1::B>);
    static_assert(awl::io::vts_read_context<OldTrivialReader, awl::io::TrivialMemoryStream, v1::B>);
    static_assert(awl::io::vts_write_context<OldTrivialWriter, awl::io::TrivialMemoryStream, v1::B>);
    static_assert(awl::io::vts_read_context<NewTrivialReader, awl::io::TrivialMemoryStream, v2::B>);

    template <class OutputStream>
    void TestWrite(const TestContext& context)
    {
        AWL_ATTRIBUTE(size_t, element_count, awl::testing::vts_common::defaultElementCount);
        AWL_ATTRIBUTE(size_t, iteration_count, 1);

        using Writer = awl::testing::vts_common::OldWriter<OutputStream>;

        const size_t mem_size = awl::testing::vts_common::MeasureStreamSize(context, element_count, false);

        OutputStream out(mem_size);

        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(iteration_count))
        {
            static_cast<void>(i);

            total_d += awl::testing::vts_common::WriteDataV1<Writer>(out, element_count, false);

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
    AWL_ATTRIBUTE(size_t, element_count, awl::testing::vts_common::defaultElementCount);
    AWL_ATTRIBUTE(size_t, write_count, 1);
    AWL_ATTRIBUTE(size_t, read_count, 1);

    AWL_ASSERT(write_count >= 1);

    const size_t mem_size = awl::testing::vts_common::MeasureStreamSize(context, element_count);

    awl::io::TrivialMemoryStream in(mem_size);
    awl::io::TrivialMemoryStream& out = in;

    AWL_ASSERT_EQUAL(mem_size, out.GetCapacity());

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(write_count))
        {
            static_cast<void>(i);

            total_d += awl::testing::vts_common::WriteDataV1<OldTrivialWriter>(out, element_count, true);

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

            total_d += awl::testing::vts_common::ReadDataPlain<OldTrivialReader>(in, element_count);
            in.Reset();
        }

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, mem_size * read_count);
    }

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);

            total_d += awl::testing::vts_common::ReadDataV1<OldTrivialReader>(in, element_count);
            in.Reset();
        }

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, mem_size * read_count);
    }

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);

            total_d += awl::testing::vts_common::ReadDataV2<NewTrivialReader>(in, element_count);
            in.Reset();
        }

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, mem_size * read_count);
    }
}

AWL_BENCHMARK(VtsMeasureSerializationInlinedVirtual)
{
    AWL_ATTRIBUTE(size_t, element_count, awl::testing::vts_common::defaultElementCount);

    using OldMeasureWriter = awl::testing::vts_common::OldWriter<awl::io::VirtualMeasureStream>;

    awl::io::VirtualMeasureStream out;

    auto d = awl::testing::vts_common::WriteDataV1<OldMeasureWriter>(out, element_count, true);

    context.logger.debug(_T("Test data has been written. "));

    helpers::ReportCountAndSpeed(context, d, element_count, out.GetLength());

    context.logger.debug(awl::format());

    AWL_ASSERT_EQUAL((awl::testing::vts_common::MeasureStreamSize(context, element_count, true)), out.GetLength());
}

AWL_BENCHMARK(VtsMeasureSerializationVirtual)
{
    AWL_ATTRIBUTE(size_t, element_count, awl::testing::vts_common::defaultElementCount);

    auto p_out = awl::io::CreateMeasureStream();

    auto d = awl::testing::vts_common::WriteDataV1<OldVirtualWriter>(*p_out, element_count, true);

    context.logger.debug(_T("Test data has been written. "));

    size_t len = (dynamic_cast<awl::io::MeasureStream&>(*p_out)).GetLength();

    helpers::ReportCountAndSpeed(context, d, element_count, len);

    context.logger.debug(awl::format());

    AWL_ASSERT_EQUAL((awl::testing::vts_common::MeasureStreamSize(context, element_count, true)), len);
}

AWL_BENCHMARK(VtsMeasureSerializationFake)
{
    AWL_ATTRIBUTE(size_t, element_count, awl::testing::vts_common::defaultElementCount);

    const size_t mem_size = awl::testing::vts_common::MeasureStreamSize(context, element_count, false);

    auto p_out = awl::io::CreateFakeStream();

    auto d = awl::testing::vts_common::WriteDataV1<OldVirtualWriter>(*p_out, element_count, true);

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
