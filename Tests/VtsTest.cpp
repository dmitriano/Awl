/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/VectorStream.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StopWatch.h"
#include "Awl/StringFormat.h"

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "Tests/Helpers/BenchmarkHelpers.h"
#include "Tests/Helpers/FormattingHelpers.h"
#include "Tests/VtsTestCommon.h"

using namespace awl::testing;
using namespace awl::testing::helpers;

namespace
{
    using OldVirtualReader = awl::testing::vts_common::OldReader<awl::io::SequentialInputStream>;
    using NewVirtualReader = awl::testing::vts_common::NewReader<awl::io::SequentialInputStream>;

    static_assert(awl::io::vts_read_context<OldVirtualReader, awl::io::SequentialInputStream, v1::B>);
    static_assert(awl::io::vts_read_context<NewVirtualReader, awl::io::SequentialInputStream, v2::B>);

    using OldVectorReader = awl::testing::vts_common::OldReader<awl::io::VectorInputStream>;
    using OldVectorWriter = awl::testing::vts_common::OldWriter<awl::io::VectorOutputStream>;
    using NewVectorReader = awl::testing::vts_common::NewReader<awl::io::VectorInputStream>;

    static_assert(awl::io::vts_read_context<OldVectorReader, awl::io::VectorInputStream, v1::B>);
    static_assert(awl::io::vts_write_context<OldVectorWriter, awl::io::VectorOutputStream, v1::B>);
    static_assert(awl::io::vts_read_context<NewVectorReader, awl::io::VectorInputStream, v2::B>);
}

AWL_TEST(VtsReadWriteVectorStream)
{
    AWL_ATTRIBUTE(size_t, element_count, awl::testing::vts_common::defaultElementCount);
    AWL_ATTRIBUTE(size_t, write_count, 1);
    AWL_ATTRIBUTE(size_t, read_count, 1);

    AWL_ASSERT(write_count >= 1);

    const size_t mem_size = awl::testing::vts_common::MeasureStreamSize(context, element_count);

    std::vector<uint8_t> v;
    v.reserve(mem_size);

    context.logger.debug(awl::format() << v.capacity() << _T(" bytes of memory has been allocated. "));

    {
        awl::io::VectorOutputStream out(v);

        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(write_count))
        {
            static_cast<void>(i);

            v.resize(0);

            total_d += awl::testing::vts_common::WriteDataV1<OldVectorWriter>(out, element_count, true);
        }

        context.logger.debug(_T("Test data has been written. "));

        helpers::ReportCountAndSpeed(context, total_d, element_count * write_count, v.size() * write_count);

        context.logger.debug(awl::format());
    }

    AWL_ASSERT_EQUAL(mem_size, v.size());
    AWL_ASSERT_EQUAL(mem_size, v.capacity());

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);

            awl::io::VectorInputStream in(v);

            total_d += awl::testing::vts_common::ReadDataPlain<OldVectorReader>(in, element_count);
        }

        context.logger.debug(_T("Plain data has been read. "));

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, v.size() * read_count);

        context.logger.debug(awl::format());
    }

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);

            awl::io::VectorInputStream in(v);

            total_d += awl::testing::vts_common::ReadDataV1<OldVectorReader>(in, element_count);
        }

        context.logger.debug(_T("Version 1 has been read. "));

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, v.size() * read_count);

        context.logger.debug(awl::format());
    }

    {
        std::chrono::steady_clock::duration total_d = std::chrono::steady_clock::duration::zero();

        for (auto i : awl::make_count(read_count))
        {
            static_cast<void>(i);

            awl::io::VectorInputStream in(v);

            total_d += awl::testing::vts_common::ReadDataV2<NewVectorReader>(in, element_count);
        }

        context.logger.debug(_T("Version 2 has been read. "));

        helpers::ReportCountAndSpeed(context, total_d, element_count * read_count, v.size() * read_count);

        context.logger.debug(awl::format());
    }
}

AWL_BENCHMARK(VtsMemSetMove)
{
    AWL_ATTRIBUTE(size_t, element_count, awl::testing::vts_common::defaultElementCount);

    std::unique_ptr<uint8_t[]> p(new uint8_t[element_count]);

    {
        context.logger.debug(_T("std::memset: "));

        awl::StopWatch w;

        std::memset(p.get(), 25u, element_count);

        helpers::ReportSpeed(context, w, element_count);

        context.logger.debug(awl::format());
    }

    std::unique_ptr<uint8_t[]> p1(new uint8_t[element_count]);

    {
        context.logger.debug(_T("std::memmove: "));

        awl::StopWatch w;

        std::memmove(p1.get(), p.get(), element_count);

        helpers::ReportSpeed(context, w, element_count);

        context.logger.debug(awl::format());
    }

    {
        context.logger.debug(_T("vector<uint8_t>::insert: "));

        std::vector<uint8_t> v;
        v.reserve(element_count);
        AWL_ASSERT_EQUAL(element_count, v.capacity());

        awl::StopWatch w;

        v.insert(v.end(), p.get(), p.get() + element_count);
        AWL_ASSERT_EQUAL(element_count, v.capacity());
        AWL_ASSERT_EQUAL(element_count, v.size());

        helpers::ReportSpeed(context, w, element_count);

        context.logger.debug(awl::format());
    }
}

namespace
{
    struct E1
    {
        std::string a;
        std::vector<int> b;
        AWL_REFLECT(a, b)
    };

    struct E2
    {
        std::vector<int> b;
        int c;
        AWL_REFLECT(b, c)
    };
}

namespace awl::mp
{
    template <>
    struct type_converter<E2>
    {
        // Make the serialization engine aware of deleted type std::string.
        using extra_tuple = std::tuple<std::string>;
    };
}

AWL_TEST(VtsDeletedType)
{
    AWL_UNUSED_CONTEXT;

    E1 e1 = { "abc", { 1, 2, 3 } };
    E2 e2 = { { 1, 2, 3 }, 1 };

    awl::io::CopyV(e1, e2);

    AWL_ASSERT(e2.b == e1.b);
    AWL_ASSERT(e2.c == 1);
}
