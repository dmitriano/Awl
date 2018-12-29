
#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <memory>
#include <functional>
#include <fstream>

#include "Awl/Io/HashStream.h"
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

typedef std::function<void(std::vector<uint8_t> &)> CorruptFunc;

template <class Hash, class T>
static void TestOnVector(const TestContext & context, Hash hash, const T & sample, const CorruptFunc & corrupt = {})
{
    AWL_ATTRIBUTE(size_t, block_size, 64);
    AWL_ATTRIBUTE(size_t, sample_count, 100);

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

            Assert::IsTrue(sample == result, _T("read/write mismatch."));
        }

        if (!corrupt)
        {
            context.out << _T(" Read speed: ");

            ReportSpeed(context, w, total_size);

            context.out << std::endl;
        }

        Assert::IsTrue(in.End());
        Assert::IsTrue(hin.End());
    }
}

typedef std::function<void(const awl::Char *)> CorruptFileFunc;

template <class Hash, class T>
static void TestOnFile(const TestContext & context, Hash hash, const T & sample, const CorruptFileFunc & corrupt = {})
{
    AWL_ATTRIBUTE(size_t, block_size, 64);
    AWL_ATTRIBUTE(size_t, sample_count, 100);
    AWL_FLAG(buffered);
    AWL_FLAG(no_hash);

    const size_t total_size = sample_count * sample.size() * sizeof(typename T::value_type);

    static const awl::Char file_name[] = _T("hash-test.dat");

    {
        std::ofstream fout;
        
        if (!buffered)
        {
            fout.rdbuf()->pubsetbuf(0, 0);
        }
        
        fout.open(file_name, std::ios::out | std::ifstream::binary);
        
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

        fin.open(file_name, std::ios::in | std::ifstream::binary);

        StdInputStream in(fin);

        HashInputStream<Hash> hin(in, block_size, hash);

        SequentialInputStream & redirected_in = no_hash ? static_cast<SequentialInputStream &>(in) : static_cast<SequentialInputStream&>(hin);

        awl::StopWatch w;

        for (size_t i = 0; i < sample_count; ++i)
        {
            T result;

            Read(redirected_in, result);

            Assert::IsTrue(sample == result, _T("read/write mismatch."));
        }

        if (!corrupt)
        {
            context.out << _T(" Read speed: ");

            ReportSpeed(context, w, total_size);

            context.out << std::endl;
        }

        Assert::IsTrue(in.End());
        Assert::IsTrue(hin.End());
    }

    //There is no wchar_t version in C++.
    //std::remove(file_name);
}

template <class Hash, class T>
static void TestCorruption(const TestContext & context, Hash hash, const T & sample, const CorruptFunc & corrupt, bool eof_allowed)
{
    AWL_ATTRIBUTE(size_t, corruption_count, 100);
    
    for (auto i : awl::make_count(corruption_count))
    {
        static_cast<void>(i);
        
        try
        {
            TestOnVector(context, hash, sample, corrupt);

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

static auto MakeVector(const TestContext & context)
{
    AWL_ATTRIBUTE(int, sample_size, 10);

    const auto sample_range = awl::make_count(sample_size);

    const std::vector<int> sample(sample_range.begin(), sample_range.end());

    return sample;
}

AWL_TEST(IoHashStreamOnVector)
{
    const std::vector<int> sample = MakeVector(context);

    awl::crypto::Crc64 hash;

    TestOnVector(context, hash, sample);
}

AWL_TEST(IoHashStreamCorruption)
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

AWL_TEST(IoHashStreamOnFile)
{
    const std::vector<int> sample = MakeVector(context);

    awl::crypto::Crc64 hash;

    TestOnFile(context, hash, sample);
}
