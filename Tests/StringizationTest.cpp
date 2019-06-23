#include "Awl/Stringizable.h"
#include "Awl/Prototype.h"
#include "Awl/Testing/UnitTest.h"

#include <string>
#include <array>
#include <sstream>
#include <any>
#include <map>

using namespace awl::testing;

struct A
{
    int x;
    double y;
    std::string z;

    AWL_STRINGIZABLE(x, y, z)
};

AWL_MEMBERWISE_EQUATABLE(A)

typedef std::vector<const char *> Vector;

void AssertMemberListEqual(const awl::helpers::MemberList ml, const Vector & v)
{
    Assert::IsTrue(v.size() == ml.size());

    for (size_t i = 0; i != v.size(); ++i)
    {
        Assert::IsTrue(v[i] == ml[i]);
    }
}

void TestMemberList(Vector v)
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
            out << ", ";
        }

        out << s;
    }

    std::string sample = out.str();

    awl::helpers::MemberList ml(sample.data());

    AssertMemberListEqual(ml, v);
}

AWT_TEST(Stringizable_MemberList)
{
    AWT_UNUSED_CONTEXT;
    
    TestMemberList({ });
    TestMemberList({ "a" });
    TestMemberList({ "aaaa", "bb", "c" });
    TestMemberList({ "x", "aaaa", "bb", "c" });

    AssertMemberListEqual(A::get_member_names(), Vector{ "x", "y", "z" });
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

    Assert::IsTrue(map.size() == 3);

    A a2 = {};

    awl::for_each_index(a2.as_tuple(), [&a2, &map](auto & val, size_t index)
    {
        val = std::any_cast<typename std::remove_reference<decltype(val)>::type>(map[a2.get_member_names()[index]]);
    });

    Assert::IsTrue(a1 == a2);
}

AWT_TEST(Stringizable_Prototype)
{
    AWT_UNUSED_CONTEXT;

    typedef std::variant<bool, char, int, float, double, std::string> V;

    awl::AttachedPrototype<V, A> ap;

    awl::DetachedPrototype dp(ap);

    awl::DetachedPrototype result(std::vector<awl::Field>{ {"x", 2u}, {"y", 4u}, {"z", 5u} });

    Assert::IsTrue(dp == result);
}
