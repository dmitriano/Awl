/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <map>

#include "Awl/TransformRange.h"

#include "Awl/String.h"

#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

AWT_TEST(TransformIterator)
{
    using TestMap = std::map<awl::String, int>;
    
    TestMap m {
        {_T("a"), 1},
        {_T("b"), 2},
        {_T("c"), 3},
    };

    auto xfunc = [](const TestMap::value_type & p) -> int { return p.second; };

    {
        using OperatorType = decltype(&decltype(xfunc)::operator());

        context.out << awl::FromACString(typeid(OperatorType).name()) << std::endl;

        static_assert(std::is_same<awl::function_traits<decltype(xfunc)>::result_type, int>::value, "function_traits error");

        context.out << awl::FromACString(typeid(awl::function_traits<decltype(xfunc)>::result_type).name()) << std::endl;

        auto begin = awl::make_transform_iterator(m.cbegin(), xfunc);

        auto end = awl::make_transform_iterator(m.cend(), xfunc);

        {
            auto i = begin;

            AWT_ASSERT(i == begin);
            AWT_ASSERT(i != end);

            AWT_ASSERT_EQUAL(1, *i++);
            AWT_ASSERT_EQUAL(2, *i++);
            AWT_ASSERT_EQUAL(3, *i++);

            AWT_ASSERT(i == end);
        }

        {
            auto i = begin;

            AWT_ASSERT(i == begin);
            AWT_ASSERT(i != end);

            AWT_ASSERT_EQUAL(1, *i);
            AWT_ASSERT_EQUAL(2, *(++i));
            AWT_ASSERT_EQUAL(3, *(++i));

            AWT_ASSERT(++i == end);
        }
    }
    
    {
        auto range = awl::make_transform_range(m, xfunc);

        int i = 0;
        
        for (auto val : range)
        {
            AWT_ASSERT_EQUAL(++i, val);
        }
    }

    auto yfunc = [](TestMap::value_type & p) -> int & { return p.second; };

    {
        using OperatorType = decltype(&decltype(yfunc)::operator());

        context.out << awl::FromACString(typeid(OperatorType).name()) << std::endl;

        auto begin = awl::make_transform_iterator(m.begin(), yfunc);

        auto end = awl::make_transform_iterator(m.end(), yfunc);

        static_assert(std::is_same<decltype(begin)::value_type, int>::value, "wrong value_type");
        static_assert(std::is_same<decltype(begin)::reference, int &>::value, "wrong value_type");
        //static_assert(std::is_same_v<decltype(&decltype(begin)::operator*()), int &>, "wrong value_type");
        static_assert(std::is_same<decltype(*begin), int &>::value, "wrong value_type");

        {
            auto i = begin;

            AWT_ASSERT(i == begin);
            AWT_ASSERT(i != end);

            AWT_ASSERT_EQUAL(1, *i++);
        
            AWT_ASSERT_EQUAL(2, *i);

            *i = 5;
            
            AWT_ASSERT_EQUAL(5, *i++);

            AWT_ASSERT_EQUAL(3, *i++);

            AWT_ASSERT(i == end);
        }

        {
            auto i = begin;

            AWT_ASSERT(i == begin);
            AWT_ASSERT(i != end);

            AWT_ASSERT_EQUAL(1, *i++);
            AWT_ASSERT_EQUAL(5, *i++);
            AWT_ASSERT_EQUAL(3, *i++);

            AWT_ASSERT(i == end);
        }
    }

    {
        auto func = [](const TestMap::value_type & p) -> const awl::String & { return p.first; };

        auto range = awl::make_transform_range(m, func);

        {
            auto i = range.begin();

            AWT_ASSERT(i == range.begin());
            AWT_ASSERT(i != range.end());

            AWT_ASSERT_EQUAL(_T("a"), *i++);
            AWT_ASSERT_EQUAL(_T("b"), *i++);
            AWT_ASSERT_EQUAL(_T("c"), *i++);

            AWT_ASSERT(i == range.end());
        }

        //(until C++20) If no captures are specified, the closure type has a defaulted copy assignment operator and a defaulted move assignment operator.
        //Otherwise, it has a deleted copy assignment operator (this includes the case when there is a capture-default, even if it does not actually capture anything).
#if AWL_CPPSTD >= 20
        {
            //This does not compile with CLang 7, but compiles with CLang 8.
            auto func1 = func;
            func = func1;
            static_cast<void>(func);
            static_cast<void>(func1);

            auto i1 = range.begin();
            auto i2 = range.begin();

            i1 = i2;

            AWT_ASSERT(i1 == i2);
        }

        {
            std::vector<awl::String> v;

            //This requires copy assignment operator to be defined.
            v.insert(v.begin(), range.begin(), range.end());

            AWT_ASSERT(v == std::vector<awl::String>{_T("a"), _T("b"), _T("c")});
        }
#endif
    }
}
