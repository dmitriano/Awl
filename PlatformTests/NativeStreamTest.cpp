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
        awl::io::UniqueStream s(awl::io::createUniqueFile(file_name));

        AWL_ASSERT(s.length() == 0);
        AWL_ASSERT(s.position() == 0);
        AWL_ASSERT(s.end());

        s.write(sample.data(), sample.size());

        AWL_ASSERT(s.length() == sample.size());

        AWL_ASSERT(s.end());
    }

    {
        awl::io::UniqueStream in(awl::io::openUniqueFile(file_name));

        AWL_ASSERT(in.length() == sample.size());
        AWL_ASSERT(in.position() == 0);

        std::vector<uint8_t> actual(sample.size());

        AWL_ASSERT(in.read(actual.data(), sample.size()) == sample.size());

        AWL_ASSERT(in.length() == sample.size());
        AWL_ASSERT(in.position() == sample.size());
        AWL_ASSERT(in.end());

        AWL_ASSERT(actual == sample);
    }

    {
        awl::io::UniqueStream s(awl::io::createUniqueFile(file_name));

        AWL_ASSERT(s.length() == sample.size());
        AWL_ASSERT(s.position() == 0);

        AWL_ATTRIBUTE(std::size_t, pos, 3);

        s.seek(pos);
        AWL_ASSERT(s.position() == pos);
        s.truncate();
        AWL_ASSERT_EQUAL(pos, s.position());
        AWL_ASSERT_EQUAL(pos, s.length());
        AWL_ASSERT(s.end());

        s.seek(0);

        std::vector<uint8_t> actual(pos);

        AWL_ASSERT(s.read(actual.data(), actual.size()) == actual.size());

        AWL_ASSERT(s.length() == actual.size());
        AWL_ASSERT(s.position() == actual.size());
        AWL_ASSERT(s.end());

        AWL_ASSERT(std::ranges::equal(sample | std::ranges::views::take(pos), actual));
    }
}

AWL_TEST(NativeStreamFileName)
{
    auto guard = awl::make_scope_guard(RemoveFile);

    awl::io::UniqueStream s(awl::io::createUniqueFile(file_name));

    context.logger->debug(_T("{}"), s.fileName());
}
