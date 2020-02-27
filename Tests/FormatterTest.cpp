#include <vector>
#include <set>

#include "Awl/Cancellation.h"
#include "Awl/Testing/Formatter.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl;
using namespace awl::testing;

template <class C>
static void TestIntContainer(const C * sample)
{
    using V = std::vector<int>;
    using F = BasicFormatter<C, V>;
    using TString = std::basic_string<C>;

    TString s_sample = sample;

    V v = F::FromString(s_sample);

    AWT_ASSERT_TRUE(v == V { AWT_LIST(10, -3, 50) });

    TString s_result = F::ToString(v);

    AWT_ASSERT_TRUE(s_sample == s_result);
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

    using V = std::set<awl::String>;
    using F = Formatter<V>;

    String s_sample = _T("a b cde");

    auto v = F::FromString(s_sample);

    AWT_ASSERT_TRUE(v == V { AWT_LIST(_T("a"), _T("b"), _T("cde")) });

    String s_result = F::ToString(v);

    AWT_ASSERT_TRUE(s_sample == s_result);
}

AWT_TEST(Formatter_BoolContainer)
{
    AWT_UNUSED_CONTEXT;

    using V = std::vector<bool>;
    using F = Formatter<V>;

    String s_sample = _T("1 0 0");

    auto v = F::FromString(s_sample);

    AWT_ASSERT_TRUE(v == V { AWT_LIST(true, false, false) });

    String s_result = F::ToString(v);

    AWT_ASSERT_TRUE(s_sample == s_result);
}

AWT_TEST(Formatter_String)
{
    AWT_UNUSED_CONTEXT;

    using F = Formatter<String>;

    String sample = _T("abc");

    auto from_result = F::FromString(sample);

    AWT_ASSERT_TRUE(from_result == sample);

    String to_result = F::ToString(sample);

    AWT_ASSERT_TRUE(to_result == sample);
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
