/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Reflection.h"
#include "Awl/Testing/UnitTest.h"

#include <string>
#include <array>
#include <sstream>
#include <any>
#include <map>

using namespace awl::testing;

namespace
{
    struct A
    {
        int x;
        double y;
        std::string z;

        AWL_REFLECT(x, y, z)
    };

    AWL_MEMBERWISE_EQUATABLE(A)

    struct B
    {
        int x;
        double y;
        std::string z;

        AWL_REFLECT(
            x,
            y,
            z)
    };

    AWL_MEMBERWISE_EQUATABLE(B)

    using Vector = std::vector<const char *>;

    void AssertMemberListEqual(const awl::helpers::MemberList ml, const Vector & v)
    {
        AWL_ASSERT(v.size() == ml.size());

        for (size_t i = 0; i != v.size(); ++i)
        {
            AWL_ASSERT(v[i] == ml[i]);
        }
    }

    void TestMemberListImpl(Vector v, const char * p_separator)
    {
        std::ostringstream out;
        bool first = true;

        for (const char * s : v)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                out << p_separator;
            }

            out << s;
        }

        std::string sample = out.str();

        awl::helpers::MemberList ml(sample.data());

        AssertMemberListEqual(ml, v);
    }

    void TestMemberList(Vector v)
    {
        TestMemberListImpl(v, ", ");
    }
}

AWL_TEST(Reflection_MemberList)
{
    AWL_UNUSED_CONTEXT;
    
    TestMemberList({ });
    TestMemberList({ "a" });
    TestMemberList({ "aaaa", "bb", "c" });
    TestMemberList({ "x", "aaaa", "bb", "c" });

    AssertMemberListEqual(A::member_names(), Vector{ "x", "y", "z" });
    AssertMemberListEqual(B::member_names(), Vector{ "x", "y", "z" });
}

AWL_TEST(Reflection_ForEach)
{
    AWL_UNUSED_CONTEXT;

    A a1 = { 1, 5.0, "abc" };

    std::map<std::string_view, std::any> map;

    awl::for_each_index(a1.as_const_tuple(), [&a1, &map](auto & val, size_t index)
    {
        map.emplace(a1.member_names()[index], val);
    });

    AWL_ASSERT(map.size() == 3);

    A a2 = {};

    awl::for_each_index(a2.as_tuple(), [&a2, &map](auto & val, size_t index)
    {
        val = std::any_cast<std::remove_reference_t<decltype(val)>>(map[a2.member_names()[index]]);
    });

    AWL_ASSERT(a1 == a2);
}

AWL_TEST(Reflection_Diff)
{
    AWL_UNUSED_CONTEXT;

    const A a1 = { 1, 5.0, "abc" };
    const A a2 = { 1, 6.0, "abc" };
    const A a3 = { 1, 6.0, "d" };

    AWL_ASSERT(std::ranges::equal(std::vector<std::string>{}, awl::different_member_names(a1, a1)));
    AWL_ASSERT(std::ranges::equal(std::vector<std::string>{"y"}, awl::different_member_names(a1, a2)));
    AWL_ASSERT(std::ranges::equal(std::vector<std::string>{"y", "z"}, awl::different_member_names(a1, a3)));
    AWL_ASSERT(std::ranges::equal(std::vector<std::string>{"z"}, awl::different_member_names(a2, a3)));
}
