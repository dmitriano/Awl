
#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <memory>
#include <functional>

#include "Awl/Io/HashStream.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Io/RwHelpers.h"
#include "Awl/Io/IoException.h"

#include "Awl/Crypto/Crc64.h"

#include "Awl/String.h"
#include "Awl/Random.h"
#include "Awl/IntRange.h"

#include "Awl/StopWatch.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;
using namespace awl::io;

typedef std::function<void(std::vector<uint8_t> &)> CorruptFunc;

static CorruptFunc no_corrupt = [](std::vector<uint8_t> &) {};

template <class Hash, class T>
static void Test(const TestContext & context, Hash hash, const T & sample, const CorruptFunc & corrupt = no_corrupt)
{
    AWL_ATTRIBUTE(size_t, block_size, 64);
    AWL_ATTRIBUTE(size_t, iteration_count, 100);

    static std::vector<uint8_t> v;

    v.resize(0);

    {
        VectorOutputStream out(v);

        HashOutputStream<Hash> hout(out, block_size, hash);

        for (int i = 0; i < iteration_count; ++i)
        {
            Write(hout, sample);
        }
    }

    corrupt(v);

    {
        VectorInputStream in(v);

        HashInputStream<Hash> hin(in, block_size, hash);

        for (int i = 0; i < iteration_count; ++i)
        {
            T result;

            Read(hin, result);

            Assert::IsTrue(sample == result, _T("read/write mismatch."));
        }

        Assert::IsTrue(in.End());
        Assert::IsTrue(hin.End());
    }
}

template <class Hash, class T>
static void TestCorruption(const TestContext & context, Hash hash, const T & sample, const CorruptFunc & corrupt, bool eof_allowed)
{
    AWL_ATTRIBUTE(size_t, corruption_count, 100);
    
    for (auto i : awl::make_count(corruption_count))
    {
        try
        {
            Test(context, hash, sample, corrupt);

            Assert::Fail(_T("Corrupted stream."));
        }
        catch (const CorruptionException &)
        {
        }
        catch (const EndOfFileException &)
        {
            Assert::IsTrue(eof_allowed, _T("End of file is not allowed."));
        }
    }
}

AWL_TEST(IoHashStream)
{
    awl::crypto::Crc64 hash;
    
    std::vector<int> sample{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    Test(context, hash, sample);

    TestCorruption(context, hash, sample, [&context](std::vector<uint8_t> & v)
    {
        std::uniform_int_distribution<size_t> dist(1, v.size() - 1);

        size_t i = dist(awl::random());

        //context.out << _T("Resizing to ") << i << _T(" original size ") << v.size() << std::endl;

        v.resize(i);
    },
    true);

    TestCorruption(context, hash, sample, [](std::vector<uint8_t> & v)
    {
        std::uniform_int_distribution<size_t> dist(0, v.size() - 1);

        size_t i = dist(awl::random());

        ++v[i];
    },
    false);
}
