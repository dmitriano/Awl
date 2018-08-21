#include "Awl/Io/RwHelpers.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Testing/UnitTest.h"

#include <iostream>
#include <algorithm>
#include <functional>

using namespace awl::testing;
using namespace awl::io;

template <class T>
static void TestObject(const TestContext & context, T sample)
{
    AWL_ATTRIBUTE(size_t, iteration_count, 10);
    
    std::vector<uint8_t> v;

    VectorOutputStream out(v);

    for (int i = 0; i < iteration_count; ++i)
    {
        Write(out, sample);
        //Serialize(out, sample, true);
    }

    VectorInputStream in(v);

    for (int i = 0; i < iteration_count; ++i)
    {
        T result;

        Read(in, result);
        //Serialize(in, result, false);

        Assert::IsTrue(sample == result);
    }

    Assert::IsTrue(in.End());
}

AWL_TEST(IoObjectReadWrite)
{
    std::hash<int> hasher;

    size_t sample = hasher(0);

    TestObject(context, sample);

    uint32_t uint_sample = static_cast<uint32_t>(sample);
    
    TestObject(context, uint_sample);

    uint8_t byte_sample = static_cast<uint8_t>(sample);

    TestObject(context, byte_sample);

    TestObject(context, std::chrono::system_clock::now());

    TestObject(context, std::string("some sample string"));
    TestObject(context, std::wstring(L"some sample string"));

    TestObject(context, std::string("a"));
    TestObject(context, std::wstring(L"a"));

    TestObject(context, std::string(""));
    TestObject(context, std::wstring(L""));

    TestObject(context, std::vector<int>{0, 1, 2, 3, 4, 5});
    TestObject(context, std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0});
    TestObject(context, std::vector<bool>{true, false, true, false, true, false});

    TestObject(context, std::vector<int>{});
    TestObject(context, std::vector<double>{});
    TestObject(context, std::vector<bool>{});
}

template <class T>
static void TestVector(const TestContext & context, std::vector<T> sample)
{
    AWL_ATTRIBUTE(size_t, iteration_count, 10);

    std::vector<uint8_t> v;

    VectorOutputStream out(v);

    for (int i = 0; i < iteration_count; ++i)
    {
        WriteVector(out, sample);
    }

    VectorInputStream in(v);

    for (int i = 0; i < iteration_count; ++i)
    {
        std::vector<T> result;

        result.resize(sample.size());

        ReadVector(in, result);

        Assert::IsTrue(sample == result);
    }

    Assert::IsTrue(in.End());
}

AWL_TEST(IoVectorReadWrite)
{
    TestVector(context, std::vector<int>{0, 1, 2, 3, 4, 5});
    TestVector(context, std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0});
    TestVector(context, std::vector<bool>{true, false, true, false, true, false});

    TestVector(context, std::vector<int>{});
    TestVector(context, std::vector<double>{});
    TestVector(context, std::vector<bool>{});
}
