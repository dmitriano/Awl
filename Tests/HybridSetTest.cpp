#include <algorithm>

#include "Awl/HybridSet.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

namespace awl
{
    class HybridSetTest
    {
    public:

        using Set = awl::hybrid_set<int>;

        void Run()
        {
            Set::Node n1{};
            n1.value = 1;
            Assert::IsTrue(set.InsertNode(&n1));
            Set::Node n4{};
            n4.value = 4;
            Assert::IsTrue(set.InsertNode(&n4));
            Set::Node n2{};
            n2.value = 2;
            Assert::IsTrue(set.InsertNode(&n2));
            Set::Node n5{};
            n5.value = 5;
            Assert::IsTrue(set.InsertNode(&n5));
            Set::Node n3{};
            n3.value = 3;
            Assert::IsTrue(set.InsertNode(&n3));
            
            Assert::IsFalse(set.InsertNode(&n1));
            Assert::IsFalse(set.InsertNode(&n2));
            Assert::IsFalse(set.InsertNode(&n3));
            Assert::IsFalse(set.InsertNode(&n4));
            Assert::IsFalse(set.InsertNode(&n5));

            Set::Node * pn3 = set.FindNodeByKey(3);
            Assert::IsTrue(pn3 != nullptr);
            Assert::IsTrue(pn3->value == 3);

            Set::Node * pn7 = set.FindNodeByKey(7);
            Assert::IsTrue(pn7 == nullptr);
        }

        Set set;
    };
}

AWT_TEST(HybridSetInternal)
{
    AWT_UNUSED_CONTEXT;

    awl::HybridSetTest test;
    test.Run();
}

