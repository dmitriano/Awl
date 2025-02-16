/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Separator.h"
#include "Awl/Testing/UnitTest.h"

namespace SeparatorTest
{
    template <class C>
    void Test()
    {
        //Why isn't it required?
        //using awl::operator<<;
        
        awl::basic_separator<C> sep(',');
        
        std::basic_ostringstream<C> out;

        out << sep << sep << sep;

        auto result = out.str();
        
        AWT_ASSERT(result == awl::StringConvertor<C>::ConvertFrom(", , "));
    }
}

AWT_TEST(Separator)
{
    AWT_UNUSED_CONTEXT;

    SeparatorTest::Test<char>();
    SeparatorTest::Test<wchar_t>();
}
