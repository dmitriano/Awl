#include "Awl/Io/RwHelpers.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Testing/UnitTest.h"

#include <iostream>
#include <algorithm>
#include <functional>

using namespace awl::testing;
using namespace awl::io;

template <class T>
static void TestScalar(const TestContext & context, T sample)
{
    AWL_ATTRIBUTE(size_t, iteration_count, 10);
    
    std::vector<uint8_t> v;

    VectorOutputStream out(v);

    for (int i = 0; i < iteration_count; ++i)
    {
        WriteScalar(out, sample);
    }

    VectorInputStream in(v);

    for (int i = 0; i < iteration_count; ++i)
    {
        T result;

        ReadScalar(in, result);

        Assert::AreEqual(sample, result);
    }

    Assert::IsTrue(in.End());
}

AWL_TEST(IoScalarReadWrite)
{
    std::hash<int> hasher;

    size_t sample = hasher(0);

    TestScalar(context, sample);

    uint32_t uint_sample = static_cast<uint32_t>(sample);
    
    TestScalar(context, uint_sample);

    uint8_t byte_sample = static_cast<uint8_t>(sample);

    TestScalar(context, byte_sample);
}

template <class Char>
static void TestString(const TestContext & context, std::basic_string<Char> sample)
{
    AWL_ATTRIBUTE(size_t, iteration_count, 10);

    std::vector<uint8_t> v;

    VectorOutputStream out(v);

    for (int i = 0; i < iteration_count; ++i)
    {
        WriteString(out, sample);
    }

    VectorInputStream in(v);

    for (int i = 0; i < iteration_count; ++i)
    {
        std::basic_string<Char> result;

        ReadString(in, result);

        Assert::IsTrue(sample == result);
    }

    Assert::IsTrue(in.End());
}

AWL_TEST(IoStringReadWrite)
{
    TestString(context, std::string("some sample string"));
    TestString(context, std::wstring(L"some sample string"));
}
