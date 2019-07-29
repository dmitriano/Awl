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

    std::vector<uint8_t> reusable_v;

    VectorOutputStream out(reusable_v);

    for (size_t i = 0; i < iteration_count; ++i)
    {
        Write(out, sample);
    }

    VectorInputStream in(reusable_v);

    for (size_t i = 0; i < iteration_count; ++i)
    {
        T result;

        Read(in, result);

        Assert::IsTrue(sample == result);
    }

    Assert::IsTrue(in.End());
}

namespace
{
    //An example of a third-party structure that we cannot change, but need to serialize.
    struct A
    {
        int x;
        double y;
    };
}

namespace awl
{
    template <>
    inline auto object_as_tuple(const A & val)
    {
        return std::tie(val.x, val.y);
    }

    template <>
    inline auto object_as_tuple(A & val)
    {
        return std::tie(val.x, val.y);
    }
}

namespace
{
    //At this point class A is already serializable, but the test code below requires it to be equatable and comparable.
    AWL_MEMBERWISE_EQUATABLE_AND_COMPARABLE(A)

    //Another option is to derive our class from A and make the derived class serializable.
    struct AWrapper : A
    {
        AWL_SERIALIZABLE(x, y)
    };

    AWL_MEMBERWISE_EQUATABLE(AWrapper)

    //Our class that we can make serializable with the single line of code.
    class B
    {
    public:

        B() : m_set{ 0, 1, 2 }, m_v{ 3, 4 }, m_a{ 'a', 'b', 'c' }, m_bm{ GameLevel::Professional }, m_bs(3ul)
        {
        }

        AWL_SERIALIZABLE(m_set, m_v, m_a, m_bm, m_bs, m_u8, m_b)

    private:

        std::set<int> m_set;
        std::vector<int> m_v;
        std::array<char, 3> m_a;

        AWL_BITMAP(GameLevel, Baby, Starter, Professional, Expert)
            GameLevelBitMap m_bm;

        std::bitset<3> m_bs;

        std::optional<uint32_t> m_u8 = 25u;

        bool m_b = true;
    };

    AWL_MEMBERWISE_EQUATABLE(B)
}

AWT_TEST(IoObjectReadWrite)
{
    {
        std::hash<int> hasher;

        const size_t sample = hasher(0);

        Test(context, sample);

        const uint32_t uint_sample = static_cast<uint32_t>(sample);

        Test(context, uint_sample);

        const uint8_t byte_sample = static_cast<uint8_t>(sample);

        Test(context, byte_sample);
    }

    Test(context, bool(true));
    Test(context, bool(false));

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

    {
        const std::vector<std::string> sv{ "a1", "b123", "c12345" };
        Test(context, sv);

        const std::vector<std::wstring> wsv{ L"a1", L"b123", L"c12345" };
        Test(context, wsv);

        std::vector<std::vector<std::string>> svv{ sv, sv, sv };
        Test(context, svv);

        std::vector<std::vector<std::wstring>> wsvv{ wsv, wsv, wsv };
        Test(context, wsvv);
    }

    {
        const std::set<int> is{ 0, 1, 2, 3, 4, 5 };

        Test(context, is);
        Test(context, std::unordered_set<int>{0, 1, 2, 3, 4, 5});

        Test(context, std::map<std::string, int>{ {"a", 0}, { "b12345", 1 }, { "c12345", 2 }, { "", 3 } });
        Test(context, std::unordered_map<std::string, int>{ {"a", 0}, { "b12345", 1 }, { "c12345", 2 }, { "", 3 } });

        Test(context, std::map<std::string, std::set<int>>{ {"a", is}, { "b12345", is }, { "c12345", is }, { "", is } });
    }

    //This test is not correct, because it passes a tuple of values by value, 
    //but if std::tuple is a member of a class the tuple of references is passed by value.
    //Test(context, std::make_tuple(5, 7.0, std::set<std::string>{"a", "b", "c"}));
}

AWT_TEST(IoPod)
{
    {
        A a1{ 5, 7.0 };
        A a2{ 5, 8.0 };

        Assert::IsTrue(a1 < a2);
    }
    
    {
        A a{ 5, 7.0 };

        A a_saved = a;

        auto a_ref = awl::object_as_tuple(a);

        ++std::get<0>(a_ref);

        Assert::IsTrue(a.x == a_saved.x + 1);

        Test(context, a);
        Test(context, std::vector<A>{a, a, a});
        Test(context, std::set<A>{a, a_saved});

        AWrapper w;
        w.x = 6;
        w.y = 8.0;
        
        Test(context, w);
    }

    {
        const B b;

        Test(context, b);
        Test(context, std::vector<B>{b, b, b});
    }
}

template <class T>
static void TestVector(const TestContext & context, std::vector<T> sample)
{
    AWL_ATTRIBUTE(size_t, iteration_count, 10);

    std::vector<uint8_t> v;

    VectorOutputStream out(v);

    for (size_t i = 0; i < iteration_count; ++i)
    {
        WriteVector(out, sample);
    }

    VectorInputStream in(v);

    for (size_t i = 0; i < iteration_count; ++i)
    {
        std::vector<T> result;

        result.resize(sample.size());

        ReadVector(in, result);

        Assert::IsTrue(sample == result);
    }

    Assert::IsTrue(in.End());
}

AWT_TEST(IoVectorReadWrite)
{
    TestVector(context, std::vector<int>{0, 1, 2, 3, 4, 5});
    TestVector(context, std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0});
    TestVector(context, std::vector<bool>{true, false, true, false, true, false});

    TestVector(context, std::vector<int>{});
    TestVector(context, std::vector<double>{});
    TestVector(context, std::vector<bool>{});
}
