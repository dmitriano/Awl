#include "Awl/HybridSet.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Random.h"
#include "Awl/String.h"

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
            std::pair<Set::iterator, bool> p = set.insert(val);
            Assert::IsTrue(p.second);
            return *(Set::ExtractListIterator(p.first));
        }

        void InsertExisting(int val)
        {
            std::pair<Set::iterator, bool> p = set.insert(val);
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

void CompareBounds(const StdSet & std_set, const MySet & my_set)
{
    Assert::AreEqual(std_set.size(), my_set.size(), _T("wrong size"));

    Assert::AreEqual(*std_set.begin(), my_set.front(), _T("wrong front"));
    Assert::AreEqual(*std_set.begin(), *my_set.begin(), _T("wrong begin"));
    Assert::AreEqual(*std_set.rbegin(), my_set.back(), _T("wrong back"));
    Assert::AreEqual(*std_set.rbegin(), *my_set.rbegin(), _T("wrong rbegin"));
}

void CompareSets(const StdSet & std_set, const MySet & my_set)
{
    {
        auto i = std_set.begin();
        
        for (auto val : my_set)
        {
            size_t std_val = *i++;
            Assert::AreEqual(std_val, val);
        }

        Assert::IsTrue(i == std_set.end());
    }
    
    {
        auto i = std_set.rbegin();
        
        for (auto my_i = my_set.rbegin(); my_i != my_set.rend(); ++my_i)
        {
            size_t std_val = *i++;
            size_t my_val = *my_i;
            Assert::AreEqual(std_val, my_val, _T("wrong element"));
        }

        Assert::IsTrue(i == std_set.rend());
    }
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

        ctx.out << val;
    }
    
    ctx.out << _T("]") << std::endl;
}

//Parameter examples:
//--filter Hybrid.* --verbose --insert_count 100 --range 10 --print_set
//--filter Hybrid.* --insert_count 1000000 --range 1000 --do_not_compare_sets
AWT_TEST(HybridSetRandom)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(size_t, range, 1000);
    AWL_FLAG(print_set);
    AWL_FLAG(do_not_compare_sets);

    std::uniform_int_distribution<size_t> dist(1, range);

    MySet my_set;
    StdSet std_set;

    for (size_t i = 0; i < insert_count; ++i)
    {
        //Insert a ramdom value.
        {
            size_t val = dist(awl::random());
            auto my_result = my_set.insert(val);
            auto std_result = std_set.insert(val);

            if (print_set)
            {
                PrintSet<awl::hybrid_set>(context, my_set);
            }

            Assert::AreEqual(std_result.second, my_result.second);
        }

        CompareBounds(std_set, my_set);

        if (!do_not_compare_sets)
        {
            CompareSets(std_set, my_set);
        }

        //Find a random value.
        {
            size_t val = dist(awl::random());
            auto my_i = my_set.find(val);
            auto std_i = std_set.find(val);
            Assert::IsTrue(my_i == my_set.end() && std_i == std_set.end() || my_i != my_set.end() && *my_i == val && std_i != std_set.end() && *std_i == val);
        }

        {
            const MySet & my_const_set = my_set;

            size_t val = dist(awl::random());
            auto my_i = my_const_set.find(val);
            auto std_i = std_set.find(val);
            Assert::IsTrue(my_i == my_const_set.end() && std_i == std_set.end() || my_i != my_const_set.end() && *my_i == val && std_i != std_set.end() && *std_i == val);
        }

        //Delete random value.
        {
            size_t val = dist(awl::random());
            bool my_result = my_set.erase(val);
            bool std_result = std_set.erase(val) != 0;

            context.out << val;
            
            if (my_result)
            {
                context.out << _T(" has been deleted.");
            }
            else
            {
                context.out << _T(" not found.");
            }

            context.out << std::endl;

            Assert::IsTrue(my_result == std_result);

            if (my_result)
            {
                if (print_set)
                {
                    PrintSet<awl::hybrid_set>(context, my_set);
                }

                CompareBounds(std_set, my_set);

                if (!do_not_compare_sets)
                {
                    CompareSets(std_set, my_set);
                }
            }
        }
    }

    my_set.clear();
}

AWT_TEST(HybridSetMove)
{
    AWT_UNUSED_CONTEXT;

    using Set = awl::hybrid_set<awl::String>;

    Set set;
    awl::String lval = _T("abc");
    set.insert(lval);
    awl::String rval = _T("xyz");
    set.insert(std::move(rval));

    Assert::IsTrue(rval == _T(""));
    Assert::IsTrue(set.front() == _T("abc"));
    Assert::IsTrue(set.back() == _T("xyz"));
}
