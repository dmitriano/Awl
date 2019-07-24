#include "Awl/HybridSet.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Random.h"

#include <algorithm>
#include <array>
#include <set>

using namespace awl::testing;

namespace awl
{
    class HybridSetTest
    {
    public:

        using Set = awl::hybrid_set<int>;

        void Run()
        {
            Set::Node * n1 = InsertNew(1);
            Set::Node * n4 = InsertNew(4);
            Set::Node * n2 = InsertNew(2);
            Set::Node * n5 = InsertNew(5);
            Set::Node * n3 = InsertNew(3);

            constexpr std::size_t count = 5;
            
            for (std::size_t i = 0; i < count; ++i)
            {
                InsertExisting(static_cast<int>(i + 1));
            }

            Set::Node * pn3 = set.FindNodeByKey(3);
            Assert::IsTrue(pn3 != nullptr);
            Assert::IsTrue(pn3->value == 3);

            Set::Node * pn7 = set.FindNodeByKey(7);
            Assert::IsTrue(pn7 == nullptr);

            {
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

            Assert::AreEqual(1, set.front());
            Assert::AreEqual(count, set.back());
            Set::Node * nN = InsertNew(284);
            Assert::AreEqual(1, set.front());
            Assert::AreEqual(nN->value, set.back());

            set.RemoveNode(n1);
            Assert::IsTrue(set.m_root == n2);
            set.RemoveNode(n2);
            Assert::IsTrue(set.m_root == n4);
            set.RemoveNode(n4);
            Assert::IsTrue(set.m_root == n5);
            set.RemoveNode(n3);
            Assert::IsTrue(set.m_root == n5);
            set.RemoveNode(n5);
            Assert::IsTrue(set.m_root == nN);
            set.RemoveNode(nN);
            Assert::IsTrue(set.m_root == nullptr);
        }

    private:

        Set::Node * InsertNew(int val)
        {
            std::pair<Set::Node *, bool> p = set.InsertNode(val);
            Assert::IsTrue(p.second);
            return p.first;
        }

        void InsertExisting(int val)
        {
            std::pair<Set::Node *, bool> p = set.InsertNode(val);
            Assert::IsFalse(p.second);
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

using MySet = awl::hybrid_set<size_t>;
using StdSet = std::set<size_t>;

void CompareSets(const StdSet & std_set, const MySet & my_set)
{
    auto i = std_set.begin();
    for (auto val : my_set)
    {
        size_t std_val = *i++;
        Assert::AreEqual(std_val, val->value);
    }

    Assert::IsTrue(i == std_set.end());
}

template <template <class> class Set>
void PrintSet(const TestContext & ctx, const Set<size_t> & set)
{
    ctx.out << _T("size: ") << set.size() << _T(" [");
    
    bool first = true;
    
    for (auto val : set)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            ctx.out << _T(", ");
        }

        ctx.out << val->value;
    }
    
    ctx.out << _T("]") << std::endl;
}

AWT_TEST(HybridSetRandom)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(size_t, range, 1000);

    std::uniform_int_distribution<size_t> dist(1, range);

    MySet my_set;
    StdSet std_set;

    for (size_t i = 0; i < insert_count; ++i)
    {
        size_t val = dist(awl::random());
        auto my_result = my_set.insert(val);
        auto std_result = std_set.insert(val);

        //PrintSet<awl::hybrid_set>(context, my_set);

        Assert::AreEqual(std_result.second, my_result.second);
        Assert::AreEqual(std_set.size(), my_set.size());
        
        const size_t my_front_val = my_set.front();
        const size_t std_front_val = *std_set.begin();
        Assert::AreEqual(std_front_val, my_front_val);

        CompareSets(std_set, my_set);
    }
}
