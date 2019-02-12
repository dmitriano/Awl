#include <vector>
#include <set>

#include "Awl/Cancellation.h"
#include "Awl/Testing/Formatter.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl;
using namespace awl::testing;

template <class C>
void TestIntContainer(const C * sample)
{
    typedef std::vector<int> V;
    typedef BasicFormatter<C, V> F;
    typedef std::basic_string<C> TString;

    TString s_sample = sample;

    V v = F::FromString(s_sample);

    Assert::IsTrue(v == V { 10, -3, 50 });

    TString s_result = F::ToString(v);

    Assert::IsTrue(s_sample == s_result);
}

AWT_TEST(Formatter_IntContainer)
{
    AWT_UNUSED_CONTEXT;

    TestIntContainer("10 -3 50");
    TestIntContainer(_T("10 -3 50"));
}

AWT_TEST(Formatter_StringContainer)
{
    AWT_UNUSED_CONTEXT;

    typedef std::set<awl::String> V;
    typedef Formatter<V> F;

    String s_sample = _T("a b cde");

    auto v = F::FromString(s_sample);

    Assert::IsTrue(v == V { _T("a"), _T("b"), _T("cde") });

    String s_result = F::ToString(v);

    Assert::IsTrue(s_sample == s_result);
}

AWT_TEST(Formatter_BoolContainer)
{
    AWT_UNUSED_CONTEXT;

    typedef std::vector<bool> V;
    typedef Formatter<V> F;

    String s_sample = _T("1 0 0");

    auto v = F::FromString(s_sample);

    Assert::IsTrue(v == V { true, false, false });

    String s_result = F::ToString(v);

    Assert::IsTrue(s_sample == s_result);
}

AWT_TEST(Formatter_String)
{
    AWT_UNUSED_CONTEXT;

    typedef Formatter<String> F;

    String sample = _T("abc");

    auto from_result = F::FromString(sample);

    Assert::IsTrue(from_result == sample);

    String to_result = F::ToString(sample);

    Assert::IsTrue(to_result == sample);
}

AWT_TEST(Formatter_Arithmetic)
{
    AWT_UNUSED_CONTEXT;

    Formatter<bool>();
    Formatter<int>();
    Formatter<long>();
    Formatter<long long>();
    Formatter<unsigned long>();
    Formatter<unsigned long long>();
    Formatter<float>();
    Formatter<double>();
    Formatter<long double>();
}
