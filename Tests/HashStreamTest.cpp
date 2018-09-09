
#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <memory>

#include "Awl/Io/HashStream.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Io/RwHelpers.h"

#include "Awl/Crypto/Crc64.h"

#include "Awl/String.h"

#include "Awl/StopWatch.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;
using namespace awl::io;

template <class Hash, class T>
static void Test(const TestContext & context, Hash hash, T sample)
{
    AWL_ATTRIBUTE(size_t, block_size, 8 + 4);

    AWL_ATTRIBUTE(size_t, iteration_count, 10);

    std::vector<uint8_t> reusable_v;

    {
        VectorOutputStream out(reusable_v);

        HashOutputStream<Hash> hout(out, block_size, hash);

        for (int i = 0; i < iteration_count; ++i)
        {
            Write(hout, sample);
        }
    }

    {
        VectorInputStream in(reusable_v);

        HashInputStream<Hash> hin(in, block_size, hash);

        for (int i = 0; i < iteration_count; ++i)
        {
            T result;

            Read(hin, result);

            Assert::IsTrue(sample == result);
        }

        Assert::IsTrue(in.End());
        Assert::IsTrue(hin.End());
    }
}

AWL_TEST(IoHashStream)
{
    awl::crypto::Crc64 hash;
    
    {
        std::vector<int> sample{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        Test(context, hash, sample);
    }
}
