/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Tests/Helpers/RwTest.h"

#include "Awl/Io/ReadWrite.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Testing/UnitTest.h"

#include <iostream>
#include <cstdint>
#include <algorithm>
#include <functional>

using namespace std::literals;

using namespace awl::testing;
using namespace awl::io;

namespace
{
    using Decimal64 = awl::decimal<uint64_t, 4>;

    enum class TestEnum { A, B, C };

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
    constexpr auto object_as_const_tuple(const A & val)
    {
        return std::tie(val.x, val.y);
    }

    template <>
    constexpr auto object_as_tuple(A & val)
    {
        return std::tie(val.x, val.y);
    }

    template <>
    struct is_tuplizable<A> : std::true_type{};
}

namespace
{
    //At this point class A is already serializable, but the test code below requires it to be equatable and comparable.
    AWL_MEMBERWISE_EQUATABLE_AND_COMPARABLE(A)

    //Another option is to derive our class from A and make the derived class serializable.
    struct AWrapper : A
    {
        AWL_TUPLIZABLE(x, y)
    };

    AWL_MEMBERWISE_EQUATABLE(AWrapper)

    //Our class that we can make serializable with the single line of code.
    class B
    {
    public:

        AWL_TUPLIZABLE(m_set, m_v, m_a, m_hset, m_bm, m_bs, m_u8, m_b, m_dec)

    private:

        std::set<int> m_set;
        std::vector<int> m_v;
        std::array<char, 3> m_a;

        awl::vector_set<int> m_hset;
        
        //It is not copyable.
        //awl::observable_set<int> m_oset;

        AWL_BITMAP(GameLevel, Baby, Starter, Professional, Expert)
        
        GameLevelBitMap m_bm;

        std::bitset<3> m_bs;

        std::optional<uint32_t> m_u8;

        bool m_b;
        
        Decimal64 m_dec;

        friend B MakeBSample();
    };

    AWL_MEMBERWISE_EQUATABLE(B)

    B MakeBSample()
    {
        B b;
        
        b.m_set = { 0, 1, 2 };
        b.m_v = { 3, 4 };
        b.m_a = { 'a', 'b', 'c' };
        b.m_hset = { 3, 4, 5 };
        //m_oset{ 6, 7, 8 };
        b.m_bm = { B::GameLevel::Professional };
        b.m_bs = 3ul;
        b.m_u8 = 25u;
        b.m_b = true;
        b.m_dec = Decimal64("123.4567890"sv);

        return b;
    }
}

AWL_TEST(IoArithmeticReadWriteLittleEndian)
{
    std::vector<uint8_t> buffer;

    {
        VectorOutputStream out(buffer);

        Write(out, uint32_t{ 0x0A0B0C0D });
        Write(out, int16_t{ -0x1234 });
        Write(out, float{ 1.0f });
        Write(out, double{ -13.25 });
    }

    const std::vector<uint8_t> expected_bytes{
        0x0D, 0x0C, 0x0B, 0x0A, // uint32_t
        0xCC, 0xED,             // int16_t
        0x00, 0x00, 0x80, 0x3F, // float
        0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x2A, 0xC0 // double
    };

    AWL_ASSERT(buffer == expected_bytes);

    VectorInputStream in(expected_bytes);

    uint32_t u32_value{};
    int16_t i16_value{};
    float f_value{};
    double d_value{};

    Read(in, u32_value);
    Read(in, i16_value);
    Read(in, f_value);
    Read(in, d_value);

    AWL_ASSERT(u32_value == 0x0A0B0C0D);
    AWL_ASSERT(i16_value == -0x1234);
    AWL_ASSERT(f_value == 1.0f);
    AWL_ASSERT(d_value == -13.25);
    AWL_ASSERT(in.End());
}

AWL_TEST(IoStdReadWrite)
{
    {
        std::hash<int> hasher;

        const size_t sample = hasher(0);

        helpers::TestReadWrite(context, sample);

        const uint32_t uint_sample = static_cast<uint32_t>(sample);

        helpers::TestReadWrite(context, uint_sample);

        const uint8_t byte_sample = static_cast<uint8_t>(sample);

        helpers::TestReadWrite(context, byte_sample);
    }

    helpers::TestReadWrite(context, bool(true));
    helpers::TestReadWrite(context, bool(false));

    helpers::TestReadWrite(context, std::chrono::system_clock::now());

    helpers::TestReadWrite(context, std::string("some sample string"));
    helpers::TestReadWrite(context, std::wstring(L"some sample string"));

    helpers::TestReadWrite(context, std::string("a"));
    helpers::TestReadWrite(context, std::wstring(L"a"));

    helpers::TestReadWrite(context, std::string(""));
    helpers::TestReadWrite(context, std::wstring(L""));

    helpers::TestReadWrite(context, std::vector<int>{0, 1, 2, 3, 4, 5});
    helpers::TestReadWrite(context, std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0});
    helpers::TestReadWrite(context, std::vector<bool>{true, false, true, false, true, false});

    helpers::TestReadWrite(context, std::vector<int>{});
    helpers::TestReadWrite(context, std::vector<double>{});
    helpers::TestReadWrite(context, std::vector<bool>{});

    {
        const std::vector<std::string> sv{ "a1", "b123", "c12345" };
        helpers::TestReadWrite(context, sv);

        const std::vector<std::wstring> wsv{ L"a1", L"b123", L"c12345" };
        helpers::TestReadWrite(context, wsv);

        std::vector<std::vector<std::string>> svv{ sv, sv, sv };
        helpers::TestReadWrite(context, svv);

        std::vector<std::vector<std::wstring>> wsvv{ wsv, wsv, wsv };
        helpers::TestReadWrite(context, wsvv);
    }

    {
        const std::set<int> is{ 0, 1, 2, 3, 4, 5 };

        helpers::TestReadWrite(context, is);
        helpers::TestReadWrite(context, std::unordered_set<int>{0, 1, 2, 3, 4, 5});

        helpers::TestReadWrite(context, std::map<std::string, int>{ {"a", 0}, { "b12345", 1 }, { "c12345", 2 }, { "", 3 } });
        helpers::TestReadWrite(context, std::unordered_map<std::string, int>{ {"a", 0}, { "b12345", 1 }, { "c12345", 2 }, { "", 3 } });

        helpers::TestReadWrite(context, std::map<std::string, std::set<int>>{ {"a", is}, { "b12345", is }, { "c12345", is }, { "", is } });
    }

    //This test is not correct, because it passes a tuple of values by value, 
    //but if std::tuple is a member of a class the tuple of references is passed by value.
    //helpers::TestReadWrite(context, std::make_tuple(5, 7.0, std::set<std::string>{"a", "b", "c"}));

    helpers::TestReadWrite(context, std::shared_ptr<std::string>{});
    helpers::TestReadWrite(context, std::make_shared<std::string>("abc"));

    helpers::TestReadWrite(context, std::unique_ptr<std::string>{});
    helpers::TestReadWrite(context, std::make_unique<std::string>("abc"));

    helpers::TestReadWrite(context, TestEnum::B);
}

AWL_TEST(IoObjectReadWrite)
{
    {
        A a1{ 5, 7.0 };
        A a2{ 5, 8.0 };

        AWL_ASSERT(a1 < a2);
    }
    
    {
        A a{ 5, 7.0 };

        A a_saved = a;

        auto a_ref = awl::object_as_tuple(a);

        ++std::get<0>(a_ref);

        AWL_ASSERT(a.x == a_saved.x + 1);

        helpers::TestReadWrite(context, a);
        helpers::TestReadWrite(context, std::vector<A>{a, a, a});
        helpers::TestReadWrite(context, std::set<A>{a, a_saved});

        AWrapper w;
        w.x = 6;
        w.y = 8.0;
        
        helpers::TestReadWrite(context, w);
    }

    {
        const B b = MakeBSample();

        helpers::TestReadWrite(context, b);
        helpers::TestReadWrite(context, std::vector<B>{b, b, b});
    }
}

AWL_TEST(IoVariantReadWrite)
{
    using V = std::variant<A, B, int>;

    helpers::TestReadWrite(context, V(A{ 5, 8.0 }));
    helpers::TestReadWrite(context, V(MakeBSample()));
    helpers::TestReadWrite(context, V(5));
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

        AWL_ASSERT(sample == result);
    }

    AWL_ASSERT(in.End());
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
