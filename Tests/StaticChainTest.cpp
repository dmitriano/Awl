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

    awl::StaticLink<X> x1("0", 0);
    awl::StaticLink<X> x2("1", 1);
    awl::StaticLink<X> x3("2", 2);

    // Declaring StaticChain as const and then doing ++i->value() in StaticChainInt test is UB.
    // const awl::StaticLink<X> x3("2", 2);
}

AWL_TEST(StaticChainInt)
{
    AWL_UNUSED_CONTEXT;

    // Test variable_static_chain()
    for (X val = 0; val < 3; ++val)
    {
        awl::StaticLink<X>* p_link = awl::variable_static_chain<X>().find_ptr(awl::aformat() << val);

        AWL_ASSERT(p_link != nullptr);
        AWL_ASSERT(p_link->value() == val);
        ++p_link->value();
    }

    for (X val = 0; val < 3; ++val)
    {
        const awl::StaticLink<X>* p_link = awl::static_chain<X>().find_ptr(awl::aformat() << val);

        AWL_ASSERT(p_link != nullptr);
        AWL_ASSERT(p_link->value() == val + 1);
    }
}

AWL_TEST(StaticChainFactoryParameterless)
{
    AWL_UNUSED_CONTEXT;

    AWL_ASSERT(awl::create<std::string>("a") == "a");
    AWL_ASSERT(awl::create<std::string>("b") == "b");
    AWL_ASSERT(awl::create<std::string>("c") == "c");

    awl::testing::Assert::Throws<awl::FactoryException>([]()
    {
        awl::create<std::string>("d");
    });
}

AWL_TEST(StaticChainFactoryArgs)
{
    AWL_UNUSED_CONTEXT;

    AWL_ASSERT(awl::create<std::string>("a1", 5) == "A5");
    AWL_ASSERT(awl::create<std::string>("b1", 7) == "B7");

    awl::testing::Assert::Throws<awl::FactoryException>([]()
    {
        // c is of different type.
        awl::create<std::string>("c", 7);
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
