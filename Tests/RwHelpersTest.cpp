#include "Awl/Io/RwHelpers.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Testing/UnitTest.h"

#include <iostream>
#include <algorithm>
#include <functional>

using namespace awl::testing;
using namespace awl::io;

template <class T>
static void Test(const TestContext & context, T sample)
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

    Test(context, sample);

    uint32_t uint_sample = static_cast<uint32_t>(sample);
    
    Test(context, uint_sample);

    uint8_t byte_sample = static_cast<uint8_t>(sample);

    Test(context, byte_sample);

    Test(context, std::chrono::system_clock::now());

    Test(context, std::string("some sample string"));
    Test(context, std::wstring(L"some sample string"));

    Test(context, std::string("a"));
    Test(context, std::wstring(L"a"));

    Test(context, std::string(""));
    Test(context, std::wstring(L""));

    Test(context, std::vector<int>{0, 1, 2, 3, 4, 5});
    Test(context, std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0});
    Test(context, std::vector<bool>{true, false, true, false, true, false});

    Test(context, std::vector<int>{});
    Test(context, std::vector<double>{});
    Test(context, std::vector<bool>{});

    std::vector<std::string> sv{"a1", "b123", "c12345"};
    Test(context, sv);

    std::vector<std::wstring> wsv{ L"a1", L"b123", L"c12345" };
    Test(context, wsv);

    std::vector<std::vector<std::string>> svv{ sv, sv, sv };
    Test(context, svv);

    std::vector<std::vector<std::wstring>> wsvv{ wsv, wsv, wsv };
    Test(context, wsvv);

    std::set<int> is{ 0, 1, 2, 3, 4, 5 };
    
    Test(context, is);
    Test(context, std::unordered_set<int>{0, 1, 2, 3, 4, 5});

    Test(context, std::map<std::string, int>{ {"a", 0}, { "b12345", 1 }, { "c12345", 2 }, { "", 3 } });
    Test(context, std::unordered_map<std::string, int>{ {"a", 0}, { "b12345", 1 }, { "c12345", 2 }, { "", 3 } });

    Test(context, std::map<std::string, std::set<int>>{ {"a", is}, { "b12345", is }, { "c12345", is }, { "", is } });
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
