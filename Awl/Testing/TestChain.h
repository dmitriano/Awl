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

#define AWT_LINK_FUNC_NAME(test_name) test_name##_TestFunc
#define AWT_LINK_FUNC_SIGNATURE(test_name) static void AWT_LINK_FUNC_NAME(test_name)(const awl::testing::TestContext& context)

//A test is simply a static function.
#define AWT_LINK(test_name, suffix) \
    AWT_LINK_FUNC_SIGNATURE(test_name); \
    static awl::testing::TestLink test_name##_##suffix_TestLink(#test_name "_" #suffix, &AWT_LINK_FUNC_NAME(test_name)); \
    AWT_LINK_FUNC_SIGNATURE(test_name)

#define AWT_DISABLED_FUNC(test_name) \
    AWT_LINK_FUNC_SIGNATURE(test_name)

#define AWT_DISABLED_TEST(test_name) \
    AWT_DISABLED_FUNC(test_name)

#define AWT_DISABLED_BENCHMARK(test_name) \
    AWT_DISABLED_FUNC(test_name)

#define AWT_DISABLED_EXAMPLE(test_name) \
    AWT_DISABLED_FUNC(test_name)

#define AWT_TEST(test_name) AWT_LINK(test_name, Test)
#define AWT_BENCHMARK(test_name) AWT_LINK(test_name, Benchmark)
#define AWT_EXAMPLE(test_name) AWT_LINK(test_name, Example)

#define AWT_UNSTABLE_TEST(test_name) AWT_LINK(test_name, Test_Unstable)
#define AWT_UNSTABLE_BENCHMARK(test_name) AWT_LINK(test_name, Benchmark_Unstable)
#define AWT_UNSTABLE_EXAMPLE(test_name) AWT_LINK(test_name, Example_Unstable)

#define AWT_UNUSED_CONTEXT static_cast<void>(context)
