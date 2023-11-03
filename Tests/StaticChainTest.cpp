/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/StaticChain.h"
#include "Awl/StaticFactory.h"
#include "Awl/StringFormat.h"

#include "Awl/Testing/UnitTest.h"

namespace
{
    using X = int;

    awl::StaticLink<X> x1(_T("0"), 0);
    awl::StaticLink<X> x2(_T("1"), 1);
    awl::StaticLink<X> x3(_T("2"), 2);
}

AWT_TEST(StaticChainInt)
{
    AWT_UNUSED_CONTEXT;

    for (X val = 0; val < 3; ++val)
    {
        auto i = awl::static_chain<X>().find(awl::format() << val);
        
        AWT_ASSERT(i != awl::static_chain<X>().end());
        AWT_ASSERT(i->value() == val);
    }
}

AWT_TEST(StaticChainFactoryParameterless)
{
    AWT_UNUSED_CONTEXT;

    AWT_ASSERT(awl::create<std::string>(_T("a")) == "a");
    AWT_ASSERT(awl::create<std::string>(_T("b")) == "b");
    AWT_ASSERT(awl::create<std::string>(_T("c")) == "c");

    awl::testing::Assert::Throws<awl::FactoryException>([]()
    {
        awl::create<std::string>(_T("d"));
    });
}

AWT_TEST(StaticChainFactoryArgs)
{
    AWT_UNUSED_CONTEXT;

    AWT_ASSERT(awl::create<std::string>(_T("a1"), 5) == "A5");
    AWT_ASSERT(awl::create<std::string>(_T("b1"), 7) == "B7");

    awl::testing::Assert::Throws<awl::FactoryException>([]()
    {
        // c is of different type.
        awl::create<std::string>(_T("c"), 7);
    });
}

// Parameterless factory

AWL_FACTORY(std::string, a)
{
    return "a";
}

AWL_FACTORY(std::string, b)
{
    return "b";
}

static std::string CreateC()
{
    return "c";
}

AWL_REGISTER_FACTORY(CreateC, c)

// Factory with parameters
namespace
{
    std::string CreateA(int val)
    {
        return awl::aformat() << "A" << val;
    }

    std::string CreateB(int val)
    {
        return awl::aformat() << "B" << val;
    }

    AWL_REGISTER_FACTORY(CreateA, a1)
        AWL_REGISTER_FACTORY(CreateB, b1)
}
