
#include <iostream>
#include <iomanip>
#include <map>

#include "Awl/TransformRange.h"

#include "Awl/String.h"

#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

AWT_TEST(TransformIterator)
{
    typedef std::map<awl::String, int> TestMap;
    
    TestMap m {
        {_T("a"), 1},
        {_T("b"), 2},
        {_T("c"), 3},
    };

    auto xfunc = [](const TestMap::value_type & p) -> int { return p.second; };

    {
        typedef decltype(&decltype(xfunc)::operator()) OperatorType;

        context.out << awl::FromACString(typeid(OperatorType).name()) << std::endl;

#if AWL_CPPSTD >= 17

        static_assert(std::is_same<awl::function_traits<decltype(xfunc)>::result_type, int>::value, "function_traits error");

        context.out << awl::FromACString(typeid(awl::function_traits<decltype(xfunc)>::result_type).name()) << std::endl;

#else
        typedef awl::return_type_t<OperatorType> result_type;
        
        static_assert(std::is_same<result_type, int>::value, "return_type_t error");

#endif
        auto begin = awl::make_transform_iterator(m.cbegin(), xfunc);

        auto end = awl::make_transform_iterator(m.cend(), xfunc);

        {
            auto i = begin;

            Assert::IsTrue(i == begin);
            Assert::IsTrue(i != end);

            Assert::AreEqual(1, *i++);
            Assert::AreEqual(2, *i++);
            Assert::AreEqual(3, *i++);

            Assert::IsTrue(i == end);
        }

        {
            auto i = begin;

            Assert::IsTrue(i == begin);
            Assert::IsTrue(i != end);

            Assert::AreEqual(1, *i);
            Assert::AreEqual(2, *(++i));
            Assert::AreEqual(3, *(++i));

            Assert::IsTrue(++i == end);
        }
    }
    
    {
        auto range = awl::make_transform_range(m, xfunc);

        int i = 0;
        
        for (auto val : range)
        {
            Assert::AreEqual(++i, val);
        }
    }

    auto yfunc = [](TestMap::value_type & p) -> int & { return p.second; };

    {
        typedef decltype(&decltype(yfunc)::operator()) OperatorType;

        context.out << awl::FromACString(typeid(OperatorType).name()) << std::endl;

        auto begin = awl::make_transform_iterator(m.begin(), yfunc);

        auto end = awl::make_transform_iterator(m.end(), yfunc);

        static_assert(std::is_same<decltype(begin)::value_type, int>::value, "wrong value_type");
        static_assert(std::is_same<decltype(begin)::reference, int &>::value, "wrong value_type");
        //static_assert(std::is_same_v<decltype(&decltype(begin)::operator*()), int &>, "wrong value_type");
        static_assert(std::is_same<decltype(*begin), int &>::value, "wrong value_type");

        {
            auto i = begin;

            Assert::IsTrue(i == begin);
            Assert::IsTrue(i != end);

            Assert::AreEqual(1, *i++);
        
            Assert::AreEqual(2, *i);

            *i = 5;
            
            Assert::AreEqual(5, *i++);

            Assert::AreEqual(3, *i++);

            Assert::IsTrue(i == end);
        }

        {
            auto i = begin;

            Assert::IsTrue(i == begin);
            Assert::IsTrue(i != end);

            Assert::AreEqual(1, *i++);
            Assert::AreEqual(5, *i++);
            Assert::AreEqual(3, *i++);

            Assert::IsTrue(i == end);
        }
    }

    {
        auto func = [](const TestMap::value_type & p) -> const awl::String & { return p.first; };

        auto range = awl::make_transform_range(m, func);

        {
            auto i = range.begin();

            Assert::IsTrue(i == range.begin());
            Assert::IsTrue(i != range.end());

            Assert::AreEqual(_T("a"), *i++);
            Assert::AreEqual(_T("b"), *i++);
            Assert::AreEqual(_T("c"), *i++);

            Assert::IsTrue(i == range.end());
        }

        {
            //This does not compile with CLang 7, but compiles with CLang 8.
            auto func1 = func;
            func = func1;

            auto i1 = range.begin();
            auto i2 = range.begin();

            i1 = i2;

            Assert::IsTrue(i1 == i2);
        }

        {
            std::vector<awl::String> v;

            //This requires copy assignment operator to be defined.
            v.insert(v.begin(), range.begin(), range.end());

            Assert::IsTrue(v == std::vector<awl::String>{_T("a"), _T("b"), _T("c")});
        }
    }
}
