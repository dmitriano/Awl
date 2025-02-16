/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <set>

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

    AWL_ASSERT((v == V { 10, -3, 50 }));

    TString s_result = F::ToString(v);

    AWL_ASSERT(s_sample == s_result);
}

AWL_TEST(Formatter_IntContainer)
{
    AWL_UNUSED_CONTEXT;

    TestIntContainer("10 -3 50");
    TestIntContainer(_T("10 -3 50"));
}

AWL_TEST(Formatter_StringContainer)
{
    AWL_UNUSED_CONTEXT;

    using V = std::set<awl::String>;
    using F = Formatter<V>;

    String s_sample = _T("a b cde");

    auto v = F::FromString(s_sample);

    AWL_ASSERT((v == V { _T("a"), _T("b"), _T("cde") }));

    String s_result = F::ToString(v);

    AWL_ASSERT(s_sample == s_result);
}

AWL_TEST(Formatter_BoolContainer)
{
    AWL_UNUSED_CONTEXT;

    using V = std::vector<bool>;
    using F = Formatter<V>;

    String s_sample = _T("1 0 0");

    auto v = F::FromString(s_sample);

    AWL_ASSERT((v == V { true, false, false }));

    String s_result = F::ToString(v);

    AWL_ASSERT(s_sample == s_result);
}

AWL_TEST(Formatter_String)
{
    AWL_UNUSED_CONTEXT;

    using F = Formatter<String>;

    String sample = _T("abc");

    auto from_result = F::FromString(sample);

    AWL_ASSERT(from_result == sample);

    String to_result = F::ToString(sample);

    AWL_ASSERT(to_result == sample);
}

AWL_TEST(Formatter_Arithmetic)
{
    AWL_UNUSED_CONTEXT;

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
