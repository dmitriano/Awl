/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/TypeProvider.h"
#include "Awl/Testing/UnitTest.h"

#include <string>
using namespace awl::testing;

namespace awl::testing
{
    class TypeProviderTest
    {
    public:

        template <class T>
        static bool tryGet(const TypeProvider& provider, T& val)
        {
            return provider.tryGet(val);
        }
    };
}

AWL_TEST(TypeProvider)
{
    AWL_UNUSED_CONTEXT;

    TypeProvider provider;

    provider.set(42);
    provider.set(std::string("abc"));

    int i = 0;
    std::string s;
    double d = 0;

    AWL_ASSERT(TypeProviderTest::tryGet(provider, i));
    AWL_ASSERT_EQUAL(42, i);

    AWL_ASSERT(TypeProviderTest::tryGet(provider, s));
    AWL_ASSERT_EQUAL(std::string("abc"), s);

    AWL_ASSERT_FALSE(TypeProviderTest::tryGet(provider, d));

    provider.set(77);

    AWL_ASSERT_EQUAL(77, provider.get<int>());

    bool thrown = false;

    try
    {
        provider.get<double>();
    }
    catch (const awl::GeneralException&)
    {
        thrown = true;
    }

    AWL_ASSERT(thrown);
}
