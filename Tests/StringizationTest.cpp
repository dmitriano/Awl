#include "Awl/Stringizable.h"
#include "Awl/Testing/UnitTest.h"

#include <string>
#include <array>
#include <sstream>

using namespace awl::testing;

struct A
{
    int x;
    double y;
    std::string z;

    AWL_STRINGIZABLE(x, y, z)
};

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

    AssertMemberListEqual(A::get_member_list(), Vector{ "x", "y", "z" });
}

//AWT_TEST(Stringizable_MemberList)
//{
//    static char va[] = "x, y, z";
//    va[5] = "a";
//}