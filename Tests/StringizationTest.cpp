#include "Awl/Stringizable.h"
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

        AWL_STRINGIZABLE(x, y, z)
    };

    AWL_MEMBERWISE_EQUATABLE(A)

    struct B
    {
        int x;
        double y;
        std::string z;

        AWL_STRINGIZABLE(
            x,
            y,
            z)
    };

    AWL_MEMBERWISE_EQUATABLE(B)

    using Vector = std::vector<const char *>;

    void AssertMemberListEqual(const awl::helpers::MemberList ml, const Vector & v)
    {
        AWT_ASSERT(v.size() == ml.size());

        for (size_t i = 0; i != v.size(); ++i)
        {
            AWT_ASSERT(v[i] == ml[i]);
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

AWT_TEST(Stringizable_MemberList)
{
    AWT_UNUSED_CONTEXT;
    
    TestMemberList({ });
    TestMemberList({ "a" });
    TestMemberList({ "aaaa", "bb", "c" });
    TestMemberList({ "x", "aaaa", "bb", "c" });

    AssertMemberListEqual(A::get_member_names(), Vector{ "x", "y", "z" });
    AssertMemberListEqual(B::get_member_names(), Vector{ "x", "y", "z" });
}

AWT_TEST(Stringizable_ForEach)
{
    AWT_UNUSED_CONTEXT;

    A a1 = { 1, 5.0, "abc" };

    std::map<std::string_view, std::any> map;

    awl::for_each_index(a1.as_const_tuple(), [&a1, &map](auto & val, size_t index)
    {
        map.emplace(a1.get_member_names()[index], val);
    });

    AWT_ASSERT(map.size() == 3);

    A a2 = {};

    awl::for_each_index(a2.as_tuple(), [&a2, &map](auto & val, size_t index)
    {
        val = std::any_cast<std::remove_reference_t<decltype(val)>>(map[a2.get_member_names()[index]]);
    });

    AWT_ASSERT(a1 == a2);
}
