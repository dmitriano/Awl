
#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <memory>

#include "Awl/Crypto/Crc64.h"

#ifdef AWL_OPENSSL
#include "Awl/Crypto/OpenSslHash.h"
#endif

#include "Awl/String.h"
#include "Awl/IntRange.h"

#include "Awl/StopWatch.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

static double ReportSpeed(const TestContext & context, const awl::StopWatch & w, size_t size)
{
    const auto time = w.GetElapsedSeconds<double>();

    const double speed = size / time / (1024 * 1024);

    context.out << std::fixed << std::setprecision(2) << speed << _T(" MB/sec");

    return speed;
}

template <class Hash>
static void CalcHash(const TestContext & context, const awl::Char * type_name = nullptr)
{
    const Hash hash;

    {
        auto r = awl::make_int_range<uint8_t>(0, 1);

        auto zero_val = hash(r.begin(), r.end());

        Assert::IsFalse(zero_val == typename Hash::value_type{});
    }

    AWL_ATTRIBUTE(size_t, vector_size, 1000000);
    AWL_ATTRIBUTE(size_t, iteration_count, 100);

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

        context.out << _T(" Hash : 0x") << std::hex;

        for (size_t i = 0; i < val.size(); ++i)
        {
            context.out << val[i];
        }

        context.out << std::endl;
    }
}

AWT_TEST(Hash)
{
    AWT_UNUSED_CONTEXT;

    using namespace awl::crypto;

    const Crc64 hash(0);

    {
        std::string sample("123456789");

        const Crc64::value_type sample_val = { 0xe9, 0xc6, 0xd9, 0x14, 0xc4, 0xb8, 0xd9, 0xca };

        const Crc64::value_type val = hash(sample.begin(), sample.end());

        Assert::IsTrue(val == sample_val);
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

    const auto a = to_array(sample);

    const auto val = from_array<uint64_t>(a);

    Assert::AreEqual(sample, val);
}

namespace examples
{
    //Theoretically as an example, StringHash can be used with switch operator, but is a bit strange usage.
    template <class Hash>
    class StringHash
    {
    public:

        static constexpr size_t size()
        {
            return Hash::size();
        }

        using value_type = typename Hash::value_type;

        explicit StringHash(Hash h = {}) : m_hash(h)
        {
        }

        template <typename C>
        value_type operator()(const std::basic_string<C> & str) const
        {
            return m_hash(str.begin(), str.end());
        }

        template <typename C, size_t N>
        constexpr value_type operator()(const C(&s)[N]) const
        {
            static_assert(N >= 1, "The parameter is not a string literal.");

            constexpr size_t length = N - 1;

            return m_hash(s, s + length);
        }

    private:

        Hash m_hash;
    };

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
}

AWT_TEST(Hash_String)
{
    AWT_UNUSED_CONTEXT;

    using namespace awl::crypto;

    using Hash = examples::StringHash<Crc64>;

    const Hash hash(0);

    {
        std::string sample("123456789");

        const Hash::value_type sample_val = { 0xe9, 0xc6, 0xd9, 0x14, 0xc4, 0xb8, 0xd9, 0xca };

        const Hash::value_type val = hash(sample);

        Assert::IsTrue(sample_val == val);
    }

    {
        std::wstring sample(L"123456789");

        auto str_hash = hash(sample);

        auto literal_hash = hash(L"123456789");

        Assert::IsTrue(str_hash == literal_hash);
    }
}

AWT_TEST(Hash_Switch)
{
    AWT_UNUSED_CONTEXT;

    using namespace awl::crypto;

    using namespace examples;

    using Hash = StringHash<EasyHash>;

    const Hash hash;

    const std::string sample = "communism";

    switch (hash(sample))
    {
    case hash("never comes"):
        Assert::Fail();

    case hash("communism"):
        break;
    
    default:
        Assert::Fail();
    }
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
