/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/NativeStream.h"
#include "Awl/Io/BufferedStream.h"

#include "Awl/String.h"

#include "Awl/Testing/UnitTest.h"

#include <filesystem>

using namespace awl::testing;

AWT_TEST(NativeStream)
{
    AWT_UNUSED_CONTEXT;

    const awl::Char file_name[] = _T("native.dat");

    std::filesystem::remove(file_name);

    const std::vector<uint8_t> sample = {'A', 'B', 'C'};

    {
        awl::io::UniqueStream out(awl::io::CreateUniqueFile(file_name));

        AWT_ASSERT(out.GetLength() == 0);
        AWT_ASSERT(out.GetPosition() == 0);
        AWT_ASSERT(out.End());

        out.Write(sample.data(), sample.size());

        AWT_ASSERT(out.GetLength() == sample.size());

        AWT_ASSERT(out.End());
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
}
