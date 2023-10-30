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

AWT_TEST(NativeStream)
{
    auto guard = awl::make_scope_guard(RemoveFile);

    const std::vector<uint8_t> sample = {'A', 'B', 'C', 'D'};

    {
        awl::io::UniqueStream s(awl::io::CreateUniqueFile(file_name));

        AWT_ASSERT(s.GetLength() == 0);
        AWT_ASSERT(s.GetPosition() == 0);
        AWT_ASSERT(s.End());

        s.Write(sample.data(), sample.size());

        AWT_ASSERT(s.GetLength() == sample.size());

        AWT_ASSERT(s.End());
    }

    {
        awl::io::UniqueStream in(awl::io::OpenUniqueFile(file_name));

        AWT_ASSERT(in.GetLength() == sample.size());
        AWT_ASSERT(in.GetPosition() == 0);

        std::vector<uint8_t> actual(sample.size());

        AWT_ASSERT(in.Read(actual.data(), sample.size()) == sample.size());

        AWT_ASSERT(in.GetLength() == sample.size());
        AWT_ASSERT(in.GetPosition() == sample.size());
        AWT_ASSERT(in.End());

        AWT_ASSERT(actual == sample);
    }

    {
        awl::io::UniqueStream s(awl::io::CreateUniqueFile(file_name));

        AWT_ASSERT(s.GetLength() == sample.size());
        AWT_ASSERT(s.GetPosition() == 0);

        AWT_ATTRIBUTE(std::size_t, pos, 3);

        s.Seek(pos);
        AWT_ASSERT(s.GetPosition() == pos);
        s.Truncate();
        AWT_ASSERT_EQUAL(pos, s.GetPosition());
        AWT_ASSERT_EQUAL(pos, s.GetLength());
        AWT_ASSERT(s.End());

        s.Seek(0);

        std::vector<uint8_t> actual(pos);

        AWT_ASSERT(s.Read(actual.data(), actual.size()) == actual.size());

        AWT_ASSERT(s.GetLength() == actual.size());
        AWT_ASSERT(s.GetPosition() == actual.size());
        AWT_ASSERT(s.End());

        AWT_ASSERT(std::ranges::equal(sample | std::ranges::views::take(pos), actual));
    }
}

AWT_TEST(NativeStreamFileName)
{
    auto guard = awl::make_scope_guard(RemoveFile);

    awl::io::UniqueStream s(awl::io::CreateUniqueFile(file_name));

    context.out << s.GetFileName() << std::endl;
}
