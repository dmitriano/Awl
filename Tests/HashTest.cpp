/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <memory>

#ifdef AWL_OPENSSL
#include "Awl/Crypto/OpenSslHash.h"
#endif

#include "Awl/String.h"
#include "Awl/IntRange.h"

#include "Awl/StopWatch.h"
#include "Awl/Crypto/Crc64.h"
#include "Awl/Crypto/FixedHash.h"

#include "Awl/Testing/UnitTest.h"

#include "Awl/Io/TypeHash.h"

#include "Helpers/BenchmarkHelpers.h"
#include "Helpers/FormattingHelpers.h"

using namespace awl::testing;
using namespace awl::testing::helpers;

using awl::crypto::Int64Hash;

namespace
{
    //StringHash can be used with switch operator.
    template <class Hash>
    using StringHash = awl::crypto::FixedHash<Hash>;

    class EasyHash
    {
    public:

        using value_type = uint32_t;

        uint32_t operator()(std::string::const_iterator begin, std::string::const_iterator end) const
        {
            return (*this)(&(*begin), &(*begin) + (end - begin));
        }

        constexpr uint32_t operator()(const char * begin, const char * end) const
        {
            uint32_t seed = std::numeric_limits<uint32_t>::max() / 2;

            for (const char * p = begin; p != end; ++p)
            {
                seed = static_cast<uint32_t>(*p * 33ULL) ^ seed;
            }

            return seed;
        }
    };

    constexpr char sampleString[] = "123456789";
    constexpr awl::crypto::HashValue<8> sampleHash = { 0xe9, 0xc6, 0xd9, 0x14, 0xc4, 0xb8, 0xd9, 0xca };
    constexpr Int64Hash::value_type sampleIntHash = 0xe9c6d914c4b8d9ca;

    static_assert(StringHash<Int64Hash>(0)(sampleString) == sampleIntHash);

    //It is not a constexpr because std::array::operator == is not constexpr in C++17, but it will be in C++20.
    //static_assert(StringHash<awl::crypto::Crc64>(0)("123456789") == sampleHash);
}

template <class Hash>
static void CalcHash(const TestContext & context, const awl::Char * type_name = nullptr)
{
    const Hash hash;

    {
        auto r = awl::make_int_range<uint8_t>(0, 1);

        auto zero_val = hash(r.begin(), r.end());

        AWT_ASSERT_FALSE(zero_val == typename Hash::value_type{});
    }

    AWT_ATTRIBUTE(size_t, vector_size, 1000000);
    AWT_ATTRIBUTE(size_t, iteration_count, 100);

    std::unique_ptr<uint8_t[]> p_buffer(new uint8_t[vector_size]);

    for (size_t i = 0; i < vector_size; ++i)
    {
        p_buffer[i] = static_cast<uint8_t>(i);
    }

    typename Hash::value_type val = {};

    {
        awl::StopWatch w;

        for (size_t i = 0; i < iteration_count; ++i)
        {
            typename Hash::value_type new_val = hash(p_buffer.get(), p_buffer.get() + vector_size);

            if (val == typename Hash::value_type{})
            {
                val = new_val;
            }
            else
            {
                Assert::IsTrue(new_val == val);
            }
        }

        if (type_name == nullptr)
        {
            context.out << awl::FromACString(typeid(hash).name());
        }
        else
        {
            context.out << type_name;
        }

        context.out << _T(": ");

        ReportSpeed(context, w, vector_size * iteration_count * sizeof(uint8_t));

        context.out << _T(" Hash : ") << val << std::endl;
    }
}

AWT_TEST(Hash)
{
    AWT_UNUSED_CONTEXT;

    using namespace awl::crypto;

    const Crc64 hash(0);

    {
        std::string sample(sampleString);

        const Crc64::value_type val = hash(sample.begin(), sample.end());

        Assert::IsTrue(val == sampleHash);
    }

    {
        std::set<int> set{ 1, 2, 3 };

        hash(set.begin(), set.end());
    }
}

AWT_BENCHMARK(HashPerformance)
{
    using namespace awl::crypto;

    CalcHash<Crc64>(context, _T("Crc64"));

#ifdef AWL_OPENSSL

    CalcHash<Md5>(context, _T("Md5"));
    CalcHash<Sha1>(context, _T("Sha1"));
    CalcHash<Sha256>(context, _T("Sha256"));
    CalcHash<Sha512>(context, _T("Sha512"));

#endif
}

AWT_TEST(Hash_ToFromArray)
{
    AWT_UNUSED_CONTEXT;

    using namespace awl::crypto;

    constexpr uint64_t sample = UINT64_C(0xab28ecb46814fe75);

    const auto a = awl::to_array(sample);

    const auto val = awl::from_array<uint64_t>(a);

    AWT_ASSERT_EQUAL(sample, val);
}

AWT_TEST(Hash_String)
{
    AWT_UNUSED_CONTEXT;

    using namespace awl::crypto;

    using Hash = StringHash<Crc64>;

    const Hash hash(0);

    {
        std::string sample(sampleString);

        const Hash::value_type val = hash(sample);

        Assert::IsTrue(val == sampleHash);
    }

    {
        std::wstring sample(L"123456789");

        auto str_hash = hash(sample);

        auto literal_hash = hash(L"123456789");

        Assert::IsTrue(str_hash == literal_hash);
    }
}

namespace
{
    template <class InternalHash>
    void TestSwitch()
    {
        using namespace awl::crypto;

        using Hash = StringHash<InternalHash>;

        constexpr Hash hash;

        const std::string sample = "communism";

        switch (hash(sample))
        {
        case hash("communism"):
            break;

        case hash("will never come"):
            AWT_FAIL;

        case hash("but the crisis"):
            AWT_FAIL;

        case hash("has begun"):
            AWT_FAIL;

        default:
            AWT_FAIL;
        }
    }
}

AWT_TEST(Hash_Switch)
{
    AWT_UNUSED_CONTEXT;

    TestSwitch<EasyHash>();
    TestSwitch<Int64Hash>();
}

AWT_TEST(Hash_Fake)
{
    AWT_UNUSED_CONTEXT;

    using namespace awl::crypto;

    const std::string sample1 = "communism";
    const std::string sample2 = "never comes";

    FakeHash hash;

    Assert::IsTrue(hash(sample1.begin(), sample1.end()) == hash(sample2.begin(), sample2.end()));
}
