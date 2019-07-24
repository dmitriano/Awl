#include <algorithm>
#include <array>

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
            Set::Node * n1 = set.InsertNode(1);
            Assert::IsTrue(n1 != nullptr);
            Set::Node * n4 = set.InsertNode(4);
            Assert::IsTrue(n4 != nullptr);
            Set::Node * n2 = set.InsertNode(2);
            Assert::IsTrue(n2 != nullptr);
            Set::Node * n5 = set.InsertNode(5);
            Assert::IsTrue(n5 != nullptr);
            Set::Node * n3 = set.InsertNode(3);
            Assert::IsTrue(n3 != nullptr);

            Assert::IsTrue(set.InsertNode(1) == nullptr);
            Assert::IsTrue(set.InsertNode(2) == nullptr);
            Assert::IsTrue(set.InsertNode(3) == nullptr);
            Assert::IsTrue(set.InsertNode(4) == nullptr);
            Assert::IsTrue(set.InsertNode(5) == nullptr);

            Set::Node * pn3 = set.FindNodeByKey(3);
            Assert::IsTrue(pn3 != nullptr);
            Assert::IsTrue(pn3->value == 3);

            Set::Node * pn7 = set.FindNodeByKey(7);
            Assert::IsTrue(pn7 == nullptr);

            {
                constexpr std::size_t count = 5;
                std::array<Set::Node *, count> nodes = { n1, n2, n3, n4, n5 };

                for (std::size_t i = 0; i < count; ++i)
                {
                    Set::Node * x = nodes[i];

                    Set::Node * predecessor = i != 0 ? nodes[i - 1] : nullptr;
                    Set::Node * successor = i != count - 1 ? nodes[i + 1] : nullptr;

                    Assert::IsTrue(set.GetPredecessor(x) == predecessor);
                    Assert::IsTrue(set.GetSuccessor(x) == successor);
                }
            }

            set.RemoveNode(n1);
            Assert::IsTrue(set.m_root == n2);
            set.RemoveNode(n2);
            Assert::IsTrue(set.m_root == n4);
            set.RemoveNode(n4);
            Assert::IsTrue(set.m_root == n5);
            set.RemoveNode(n3);
            Assert::IsTrue(set.m_root == n5);
            set.RemoveNode(n5);
            Assert::IsTrue(set.m_root == nullptr);
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

