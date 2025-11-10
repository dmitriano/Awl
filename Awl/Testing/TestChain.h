/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/SingleList.h"
#include "Awl/String.h"
#include "Awl/StaticChain.h"
#include "Awl/Testing/TestContext.h"

#include <type_traits>

namespace awl::testing
{
    using TestFunc = const std::add_pointer_t<void(const TestContext& context)>;

    //The objects of this class are static, they are not supposed to be created on the heap or on the stack.
    //There is no safe_exclude() in the destructor, because the order of static objects destruction in undefined,
    //so the object is not excluded from the list automatically before destruction.
    using TestLink = StaticLink<TestFunc>;
    using TestChain = StaticChain<TestFunc>;
}

#define AWL_LINK_FUNC_NAME(test_name) test_name##_TestFunc
#define AWL_LINK_FUNC_SIGNATURE(test_name) static void AWL_LINK_FUNC_NAME(test_name)(const awl::testing::TestContext& context)

//A test is simply a static function.
#define AWL_LINK(test_name, suffix) \
    AWL_LINK_FUNC_SIGNATURE(test_name); \
    static awl::testing::TestLink test_name##_##suffix_TestLink(#test_name "_" #suffix, &AWL_LINK_FUNC_NAME(test_name)); \
    AWL_LINK_FUNC_SIGNATURE(test_name)

#define AWL_DISABLED_FUNC(test_name) \
    AWL_LINK_FUNC_SIGNATURE(test_name)

#define AWL_DISABLED_TEST(test_name) \
    AWL_DISABLED_FUNC(test_name)

#define AWL_DISABLED_BENCHMARK(test_name) \
    AWL_DISABLED_FUNC(test_name)

#define AWL_DISABLED_EXAMPLE(test_name) \
    AWL_DISABLED_FUNC(test_name)

#define AWL_TEST(test_name) AWL_LINK(test_name, Test)
#define AWL_BENCHMARK(test_name) AWL_LINK(test_name, Benchmark)
#define AWL_EXAMPLE(test_name) AWL_LINK(test_name, Example)
#define AWL_UTILITY(test_name) AWL_LINK(test_name, Utility)

#define AWL_UNSTABLE_TEST(test_name) AWL_LINK(test_name, Test_Unstable)
#define AWL_UNSTABLE_BENCHMARK(test_name) AWL_LINK(test_name, Benchmark_Unstable)
#define AWL_UNSTABLE_EXAMPLE(test_name) AWL_LINK(test_name, Example_Unstable)
#define AWL_UNSTABLE_UTILITY(test_name) AWL_LINK(test_name, Utility_Unstable)

#define AWL_UNUSED_CONTEXT static_cast<void>(context)

namespace awl::testing
{
    class Test
    {
    public:

        explicit Test(const awl::testing::TestContext& context) : context(context) {}

    protected:

        const awl::testing::TestContext& context;
    };
}

#define AWL_TEST_CLASS(test_class_name) \
AWL_TEST(test_class_name) \
{ \
    test_class_name{ context }.run(); \
}
