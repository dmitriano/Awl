/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/NativeStream.h"
#include "Awl/Io/BufferedStream.h"

#include "Awl/String.h"
#include "Awl/ScopeGuard.h"

#include "Awl/Testing/UnitTest.h"

#include <filesystem>
#include <ranges>

using namespace awl::testing;

namespace
{
    const awl::Char file_name[] = _T("native.dat");

    void RemoveFile()
    {
        std::filesystem::remove(file_name);
    }
}

AWL_TEST(NativeStream)
{
    auto guard = awl::make_scope_guard(RemoveFile);

    const std::vector<uint8_t> sample = {'A', 'B', 'C', 'D'};

    {
        awl::io::UniqueStream s(awl::io::CreateUniqueFile(file_name));

        AWL_ASSERT(s.GetLength() == 0);
        AWL_ASSERT(s.GetPosition() == 0);
        AWL_ASSERT(s.End());

        s.Write(sample.data(), sample.size());

        AWL_ASSERT(s.GetLength() == sample.size());

        AWL_ASSERT(s.End());
    }

    {
        awl::io::UniqueStream in(awl::io::OpenUniqueFile(file_name));

        AWL_ASSERT(in.GetLength() == sample.size());
        AWL_ASSERT(in.GetPosition() == 0);

        std::vector<uint8_t> actual(sample.size());

        AWL_ASSERT(in.Read(actual.data(), sample.size()) == sample.size());

        AWL_ASSERT(in.GetLength() == sample.size());
        AWL_ASSERT(in.GetPosition() == sample.size());
        AWL_ASSERT(in.End());

        AWL_ASSERT(actual == sample);
    }

    {
        awl::io::UniqueStream s(awl::io::CreateUniqueFile(file_name));

        AWL_ASSERT(s.GetLength() == sample.size());
        AWL_ASSERT(s.GetPosition() == 0);

        AWL_ATTRIBUTE(std::size_t, pos, 3);

        s.Seek(pos);
        AWL_ASSERT(s.GetPosition() == pos);
        s.Truncate();
        AWL_ASSERT_EQUAL(pos, s.GetPosition());
        AWL_ASSERT_EQUAL(pos, s.GetLength());
        AWL_ASSERT(s.End());

        s.Seek(0);

        std::vector<uint8_t> actual(pos);

        AWL_ASSERT(s.Read(actual.data(), actual.size()) == actual.size());

        AWL_ASSERT(s.GetLength() == actual.size());
        AWL_ASSERT(s.GetPosition() == actual.size());
        AWL_ASSERT(s.End());

        AWL_ASSERT(std::ranges::equal(sample | std::ranges::views::take(pos), actual));
    }
}

AWL_TEST(NativeStreamFileName)
{
    auto guard = awl::make_scope_guard(RemoveFile);

    awl::io::UniqueStream s(awl::io::CreateUniqueFile(file_name));

    context.logger.debug(awl::format() << s.GetFileName());
}
