#include <vector>
#include <set>

#include "Awl/Cancellation.h"
#include "Awl/Formatter.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl;
using namespace awl::testing;

template <class C>
void TestIntContainer(const C * sample)
{
    typedef std::vector<int> V;
    typedef BasicFormatter<C, V> F;
    typedef std::basic_string<C> String;

    String s_sample = sample;

    V v = F::FromString(s_sample);

    Assert::IsTrue(v == V { 10, -3, 50 });

    String s_result = F::ToString(v);

    Assert::IsTrue(s_sample == s_result);
}

AWL_TEST(Formatter_IntContainer)
{
    TestIntContainer("10 -3 50");
    TestIntContainer(_T("10 -3 50"));
}

AWL_TEST(Formatter_StringContainer)
{
    typedef std::set<awl::String> V;
    typedef Formatter<V> F;

    String s_sample = _T("a b cde");

    auto v = F::FromString(s_sample);

    Assert::IsTrue(v == V { _T("a"), _T("b"), _T("cde") });

    String s_result = F::ToString(v);

    Assert::IsTrue(s_sample == s_result);
}

AWL_TEST(Formatter_BoolContainer)
{
    typedef std::vector<bool> V;
    typedef Formatter<V> F;

    String s_sample = _T("1 0 0");

    auto v = F::FromString(s_sample);

    Assert::IsTrue(v == V { true, false, false });

    String s_result = F::ToString(v);

    Assert::IsTrue(s_sample == s_result);
}

AWL_TEST(Formatter_String)
{
    typedef Formatter<String> F;

    String sample = _T("abc");

    auto from_result = F::FromString(sample);

    Assert::IsTrue(from_result == sample);

    String to_result = F::ToString(sample);

    Assert::IsTrue(to_result == sample);
}
