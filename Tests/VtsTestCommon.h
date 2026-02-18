/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Vts.h"
#include "Awl/Io/PlainReader.h"
#include "Awl/Io/MeasureStream.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StopWatch.h"
#include "Awl/IntRange.h"
#include "Awl/StringFormat.h"
#include "Tests/VtsData.h"

#include <chrono>
#include <tuple>

namespace awl::testing::vts_common
{
    namespace vts_data = awl::testing::helpers;

    template <class IStream>
    using OldReader = awl::io::Reader<vts_data::V1, IStream>;

    template <class OStream>
    using OldWriter = awl::io::Writer<vts_data::V1, OStream>;

    template <class IStream>
    using NewReader = awl::io::Reader<vts_data::V2, IStream>;

    inline constexpr size_t defaultElementCount = 1000;

    using Duration = std::chrono::steady_clock::duration;

    template <class Writer>
    Duration WriteDataV1(typename Writer::OutputStream& out, size_t element_count, bool with_metadata)
    {
        Writer ctx;

        {
            auto& a1_proto = ctx.template FindNewPrototype<vts_data::v1::A>();
            AWL_ASSERT(a1_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<vts_data::v1::A>::Tie>);

            auto& b1_proto = ctx.template FindNewPrototype<vts_data::v1::B>();
            AWL_ASSERT(b1_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<vts_data::v1::B>::Tie>);
        }

        if (with_metadata)
        {
            ctx.WriteNewPrototypes(out);
        }

        awl::StopWatch w;

        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);
            ctx.WriteV(out, vts_data::v1::a_expected);
            ctx.WriteV(out, vts_data::v1::b_expected);
        }

        return w;
    }

    template <class Reader>
    Duration ReadDataPlain(typename Reader::InputStream& in, size_t element_count)
    {
        {
            // Skip metadata.
            Reader ctx;
            ctx.ReadOldPrototypes(in);
        }

        awl::io::PlainReader<typename Reader::Variant, typename Reader::InputStream> ctx;

        awl::StopWatch w;

        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            vts_data::v1::A a1;
            ctx.ReadV(in, a1);
            AWL_ASSERT(a1 == vts_data::v1::a_expected);

            vts_data::v1::B b1;
            ctx.ReadV(in, b1);
            AWL_ASSERT(b1 == vts_data::v1::b_expected);
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

            vts_data::v1::A a1;
            ctx.ReadV(in, a1);
            AWL_ASSERT(a1 == vts_data::v1::a_expected);

            vts_data::v1::B b1;
            ctx.ReadV(in, b1);
            AWL_ASSERT(b1 == vts_data::v1::b_expected);
        }

        AWL_ASSERT(in.End());

        return w;
    }

    template <class Reader>
    Duration ReadDataV2(typename Reader::InputStream& in, size_t element_count)
    {
        Reader ctx;
        ctx.ReadOldPrototypes(in);

        {
            auto& a2_proto = ctx.template FindNewPrototype<vts_data::v2::A>();
            AWL_ASSERT(a2_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<vts_data::v2::A>::Tie>);

            auto& b2_proto = ctx.template FindNewPrototype<vts_data::v2::B>();
            AWL_ASSERT(b2_proto.GetCount() == std::tuple_size_v<awl::tuplizable_traits<vts_data::v2::B>::Tie>);
        }

        awl::StopWatch w;

        for (size_t i : awl::make_count(element_count))
        {
            static_cast<void>(i);

            vts_data::v2::A a2;
            ctx.ReadV(in, a2);
            AWL_ASSERT(a2 == vts_data::v2::a_expected);

            // Version 1 data has v2::B so the condition is true.
            vts_data::v2::B b2;
            ctx.ReadV(in, b2);
            AWL_ASSERT(b2 == vts_data::v2::b_expected);

            // An example of how to read data that may not exist in a previous version.
            if (false)
            {
                vts_data::v2::C c2;
                ctx.ReadV(in, c2);
                AWL_ASSERT(c2 == vts_data::v2::c_expected);
            }
        }

        AWL_ASSERT(in.End());

        return w;
    }

    inline size_t MeasureStreamSize(const TestContext& context, size_t element_count, bool include_meta = true)
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

        if (include_meta)
        {
            context.logger.debug(awl::format() << _T("Meta size: ") << meta_size << _T(", "));
        }

        context.logger.debug(awl::format() << _T("block size: ") << block_size << _T(", allocating ") << mem_size << _T(" bytes of memory."));

        return mem_size;
    }
}
