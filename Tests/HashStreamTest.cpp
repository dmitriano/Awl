
#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <memory>
#include <functional>
#include <fstream>

#include "Awl/Io/HashStream.h"
#include "Awl/Io/BufferedStream.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Io/StdStream.h"
#include "Awl/Io/RwHelpers.h"
#include "Awl/Io/IoException.h"

#include "Awl/Crypto/Crc64.h"

#include "Awl/String.h"
#include "Awl/Random.h"
#include "Awl/IntRange.h"

#include "Awl/StopWatch.h"
#include "Awl/Testing/UnitTest.h"

#include "BenchmarkHelpers.h"

using namespace awl::testing;
using namespace awl::io;

using CorruptFunc = std::function<void(std::vector<uint8_t> &)>;

template <class Hash, class T>
static void TestOnVector(const TestContext & context, Hash hash, const T & sample, const CorruptFunc & corrupt = {})
{
    AWT_ATTRIBUTE(size_t, block_size, 64);
    AWT_ATTRIBUTE(size_t, sample_count, 100);

    static std::vector<uint8_t> v;

    const size_t total_size = sample_count * sample.size() * sizeof(typename T::value_type);

    v.reserve(total_size * 2);
    v.resize(0);

    {
        VectorOutputStream out(v);

        HashOutputStream<Hash> hout(out, block_size, hash);

        awl::StopWatch w;

        for (size_t i = 0; i < sample_count; ++i)
        {
            Write(hout, sample);
        }

        if (!corrupt)
        {
            context.out << _T("Write speed: ");

            ReportSpeed(context, w, total_size);
        }
    }

    if (corrupt)
    {
        corrupt(v);
    }

    {
        VectorInputStream in(v);

        HashInputStream<Hash> hin(in, block_size, hash);

        awl::StopWatch w;

        for (size_t i = 0; i < sample_count; ++i)
        {
            T result;

            Read(hin, result);

            AWT_ASSERTM_TRUE(sample == result, _T("read/write mismatch."));
        }

        if (!corrupt)
        {
            context.out << _T(" Read speed: ");

            ReportSpeed(context, w, total_size);

            context.out << std::endl;
        }

        AWT_ASSERT_TRUE(in.End());
        AWT_ASSERT_TRUE(hin.End());
    }
}

using CorruptFileFunc = std::function<void(const awl::Char *)>;

template <class Hash, class T>
static void TestOnFile(const TestContext & context, Hash hash, const T & sample, const CorruptFileFunc & corrupt = {})
{
    AWT_ATTRIBUTE(size_t, block_size, 64);
    AWT_ATTRIBUTE(size_t, sample_count, 100);
    AWT_FLAG(buffered);
    AWT_FLAG(no_hash);

    const size_t total_size = sample_count * sample.size() * sizeof(typename T::value_type);

    static const awl::Char file_name[] = _T("hash-test.dat");

    {
        std::ofstream fout;
        
        if (!buffered)
        {
            fout.rdbuf()->pubsetbuf(0, 0);
        }
        
        fout.open(file_name, std::ios::out | std::ostream::binary);
        
        StdOutputStream out(fout);

        HashOutputStream<Hash> hout(out, block_size, hash);

        SequentialOutputStream & redirected_out = no_hash ? static_cast<SequentialOutputStream &>(out) : static_cast<SequentialOutputStream &>(hout);
        
        awl::StopWatch w;

        for (size_t i = 0; i < sample_count; ++i)
        {
            Write(redirected_out, sample);
        }

        if (!corrupt)
        {
            context.out << _T("Write speed: ");

            ReportSpeed(context, w, total_size);
        }
    }

    if (corrupt)
    {
        corrupt(file_name);
    }

    {
        std::ifstream fin;

        if (!buffered)
        {
            fin.rdbuf()->pubsetbuf(0, 0);
        }

        fin.open(file_name, std::ios::in | std::istream::binary);

        StdInputStream in(fin);

        HashInputStream<Hash> hin(in, block_size, hash);

        SequentialInputStream & redirected_in = no_hash ? static_cast<SequentialInputStream &>(in) : static_cast<SequentialInputStream&>(hin);

        awl::StopWatch w;

        for (size_t i = 0; i < sample_count; ++i)
        {
            T result;

            Read(redirected_in, result);

            AWT_ASSERTM_TRUE(sample == result, _T("read/write mismatch."));
        }

        if (!corrupt)
        {
            context.out << _T(" Read speed: ");

            ReportSpeed(context, w, total_size);

            context.out << std::endl;
        }

        AWT_ASSERT_TRUE(in.End());
        AWT_ASSERT_TRUE(hin.End());
    }

    //There is no wchar_t version in C++.
    //std::remove(file_name);
}

template <class Hash, class T>
static void TestCorruption(const TestContext & context, Hash hash, const T & sample, const CorruptFunc & corrupt, bool eof_allowed)
{
    AWT_ATTRIBUTE(size_t, corruption_count, 100);
    
    for (auto i : awl::make_count(corruption_count))
    {
        static_cast<void>(i);
        
        try
        {
            TestOnVector(context, hash, sample, corrupt);

            AWT_FAIL("Corrupted stream.");
        }
        catch (const CorruptionException &)
        {
        }
        catch (const EndOfFileException &)
        {
            AWT_ASSERTM_TRUE(eof_allowed, _T("End of file is not allowed."));
        }
    }
}

static auto MakeVector(const TestContext & context)
{
    AWT_ATTRIBUTE(int, sample_size, 10);

    const auto range = awl::make_count(sample_size);

    return std::vector<int>(range.begin(), range.end());
}

AWT_TEST(IoHashStreamCorruption)
{
    const std::vector<int> sample = MakeVector(context);

    awl::crypto::Crc64 hash;

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

AWT_TEST(IoHashStreamOnVectorCrc64)
{
    TestOnVector(context, awl::crypto::Crc64(), MakeVector(context));
}

AWT_TEST(IoHashStreamOnVectorFake)
{
    TestOnVector(context, awl::crypto::FakeHash(), MakeVector(context));
}

//This test can be used to measure the disk speed
//Without hash:
//./AwlTest --filter IoHashStreamOnFile_Test --verbose --sample_count 1000000 --block_size 128000 --no_hash --buffered
//With hash:
//./AwlTest --filter IoHashStreamOnFile_Test --verbose --sample_count 1000000 --block_size 128000
AWT_TEST(IoHashStreamOnFileCrc64)
{
    TestOnFile(context, awl::crypto::Crc64(), MakeVector(context));
}

//on SSD:
//./AwlTest --filter IoHash.*FileCrc.* --verbose --block_size 100000 --sample_count 1000 --sample_size 100000
// Write speed: 327.32 MB/sec Read speed: 347.95 MB/sec
//./AwlTest --filter IoHash.*FileFake.* --verbose --block_size 100000 --sample_count 1000 --sample_size 100000
// Write speed: 1041.22 MB/sec Read speed: 1878.19 MB/sec
AWT_TEST(IoHashStreamOnFileFake)
{
    TestOnFile(context, awl::crypto::FakeHash(), MakeVector(context));
}
