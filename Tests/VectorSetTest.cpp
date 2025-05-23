/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/VectorSet.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Random.h"
#include "Awl/String.h"
#include "Awl/KeyCompare.h"
#include "Awl/Tuplizable.h"

#include <algorithm>
#include <array>
#include <set>
#include <ranges>

using namespace awl::testing;

//Check if it satisfies the concept std::ranges::range.
static_assert(std::ranges::range<awl::vector_set<int>>);

namespace awl
{
    class VectorSetTest
    {
    public:

        using Set = awl::vector_set<int>;

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

            Set::Node * pn3 = set.m_tree.FindNodeByKey(3);
            AWL_ASSERT(pn3 != nullptr);
            AWL_ASSERT(pn3->value() == 3);

            Set::Node * pn7 = set.m_tree.FindNodeByKey(7);
            AWL_ASSERT(pn7 == nullptr);

            {
                std::array<Set::Node *, count> nodes = { n1, n2, n3, n4, n5 };

                for (std::size_t i = 0; i < count; ++i)
                {
                    Set::Node * x = nodes[i];

                    Set::Node * predecessor = i != 0 ? nodes[i - 1] : nullptr;
                    Set::Node * successor = i != count - 1 ? nodes[i + 1] : nullptr;

                    AWL_ASSERT(set.m_tree.GetPredecessor(x) == predecessor);
                    AWL_ASSERT(set.m_tree.GetSuccessor(x) == successor);
                }
            }

            AWL_ASSERT_EQUAL(1, set.front());
            AWL_ASSERT_EQUAL(static_cast<int>(count), set.back());
            Set::Node * nN = InsertNew(284);
            AWL_ASSERT_EQUAL(1, set.front());
            AWL_ASSERT_EQUAL(nN->value(), set.back());

            set.m_tree.RemoveNode(n1);
            AWL_ASSERT(set.m_tree.m_root == n2);
            set.m_tree.RemoveNode(n2);
            AWL_ASSERT(set.m_tree.m_root == n4);
            set.m_tree.RemoveNode(n4);
            AWL_ASSERT(set.m_tree.m_root == n5);
            set.m_tree.RemoveNode(n3);
            AWL_ASSERT(set.m_tree.m_root == n5);
            set.m_tree.RemoveNode(n5);
            AWL_ASSERT(set.m_tree.m_root == nN);
            set.m_tree.RemoveNode(nN);
            AWL_ASSERT(set.m_tree.m_root == nullptr);
        }

    private:

        Set::Node * InsertNew(int val)
        {
            std::pair<Set::iterator, bool> p = set.insert(val);
            AWL_ASSERT(p.second);
            return *p.first.m_i;
        }

        void InsertExisting(int val)
        {
            std::pair<Set::iterator, bool> p = set.insert(val);
            AWL_ASSERT_FALSE(p.second);
        }

        Set set;
    };
}

AWL_TEST(VectorSetInternal)
{
    AWL_UNUSED_CONTEXT;

    awl::VectorSetTest test;
    test.Run();
}

namespace
{
    static size_t memory_size = 0;

    template <class T>
    class TestAllocator
    {
    public:

        using value_type = T;

        TestAllocator() = default;

        template <class Q>
        TestAllocator(const TestAllocator<Q> & other) : m_alloc(other.m_alloc)
        {
        }

        T* allocate(std::size_t n)
        {
            memory_size += n * sizeof(T);
            return m_alloc.allocate(n);
        }

        void deallocate(T* p, std::size_t n)
        {
            memory_size -= n * sizeof(T);
            return m_alloc.deallocate(p, n);
        }

    private:

        std::allocator<T> m_alloc;

        template <class Q>
        friend class TestAllocator;
    };

    using MySet = awl::vector_set<size_t, std::less<>, TestAllocator<size_t>>;
    using StdSet = std::set<size_t>;

    static void CompareBounds(const StdSet & std_set, const MySet & my_set)
    {
        const size_t size = my_set.size();
        
        AWL_ASSERTM_EQUAL(std_set.size(), size, _T("wrong size"));

        if (size != 0)
        {
            AWL_ASSERTM_EQUAL(*std_set.begin(), my_set.front(), _T("wrong front"));
            AWL_ASSERTM_EQUAL(*std_set.begin(), *my_set.begin(), _T("wrong begin"));
            AWL_ASSERTM_EQUAL(*std_set.rbegin(), my_set.back(), _T("wrong back"));
            AWL_ASSERTM_EQUAL(*std_set.rbegin(), *my_set.rbegin(), _T("wrong rbegin"));
        }
    }

    static void CompareSets(const StdSet & std_set, const MySet & my_set)
    {
        if (std_set.empty() && my_set.empty())
        {
            return;
        }
        
        {
            auto i = std_set.begin();

            for (auto val : my_set)
            {
                size_t std_val = *i++;
                AWL_ASSERT_EQUAL(std_val, val);
            }

            AWL_ASSERT(i == std_set.end());
        }

        {
            auto i = std_set.rbegin();

            for (auto my_i = my_set.rbegin(); my_i != my_set.rend(); ++my_i)
            {
                size_t std_val = *i++;
                size_t my_val = *my_i;
                AWL_ASSERTM_EQUAL(std_val, my_val, _T("wrong element"));
            }

            AWL_ASSERT(i == std_set.rend());
        }
    }

    static void PrintSet(const TestContext & ctx, const MySet & set)
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
}

//Parameter examples:
//--filter Hybrid.* --verbose --insert_count 100 --range 10 --print_set
//--filter Hybrid.* --insert_count 1000000 --range 1000 --do_not_compare_sets
AWL_TEST(VectorSetRandom)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(size_t, range, 1000);
    AWL_FLAG(print_set);
    AWL_FLAG(do_not_compare_sets);

    std::uniform_int_distribution<size_t> dist(1, range);

    MySet my_set;
    AWL_ASSERT(my_set.empty());
    AWL_ASSERT(my_set.size() == 0);

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
                PrintSet(context, my_set);
            }

            AWL_ASSERT_EQUAL(std_result.second, my_result.second);
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
            AWL_ASSERT((my_i == my_set.end() && std_i == std_set.end()) || (my_i != my_set.end() && *my_i == val && std_i != std_set.end() && *std_i == val));
        }

        {
            const MySet & my_const_set = my_set;

            size_t val = dist(awl::random());
            auto my_i = my_const_set.find(val);
            auto std_i = std_set.find(val);
            AWL_ASSERT((my_i == my_const_set.end() && std_i == std_set.end()) || (my_i != my_const_set.end() && *my_i == val && std_i != std_set.end() && *std_i == val));
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

            AWL_ASSERT(my_result == std_result);

            if (my_result)
            {
                if (print_set)
                {
                    PrintSet(context, my_set);
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
    AWL_ASSERT(my_set.empty());
    AWL_ASSERT(my_set.size() == 0);
    AWL_ASSERTM_EQUAL(0u, memory_size, _T("memory leaks"));
}

AWL_TEST(VectorSetRValue)
{
    AWL_UNUSED_CONTEXT;

    using Set = awl::vector_set<awl::String>;

    Set set;
    awl::String lval = _T("abc");
    set.insert(lval);
    awl::String rval = _T("xyz");
    set.insert(std::move(rval));
    set.emplace(_T("def"));

    AWL_ASSERT(rval == _T(""));
    AWL_ASSERT(set.front() == _T("abc"));
    AWL_ASSERT(*(++set.begin()) == _T("def"));
    AWL_ASSERT(set.back() == _T("xyz"));
}

template <class Key, class T = Key, class Compare = std::less<>>
static awl::vector_set<T, Compare> GenerateIntSet(size_t insert_count, Key range, Compare comp = {})
{
    awl::vector_set<T, Compare> sample{comp};

    std::uniform_int_distribution<Key> dist(1, range);

    for (size_t i = 0; i < insert_count; ++i)
    {
        Key val = dist(awl::random());
        sample.emplace(val);
    }

    return sample;
}

AWL_TEST(VectorSetCopyMove)
{
    AWL_UNUSED_CONTEXT;

    using Set = awl::vector_set<int>;
    
    const Set sample = GenerateIntSet<int>(1000, 1000);
    AWL_ASSERT((sample != Set{ -1, -2, -3, -4, -5 }));

    const Set copy = sample;
    AWL_ASSERT(copy == sample);

    Set temp = copy;
    AWL_ASSERT(temp == copy);

    const Set moved = std::move(temp);
    AWL_ASSERT(moved == copy);
    AWL_ASSERT(temp.empty());
}

//--filter VectorSetIndex_Test --insert_count 10000000 --range 100000000
AWL_TEST(VectorSetIndex)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(size_t, range, 1000);

    auto set = GenerateIntSet(insert_count, range);

    size_t index = 0;

    for (auto i = set.begin(); i != set.end(); ++i)
    {
        auto val = *i;

        const auto found_val = set.at(index);
        AWL_ASSERT_EQUAL(val, found_val);

        auto [found_iter, found_index] = set.find2(val);
        AWL_ASSERT_EQUAL(index, found_index);
        AWL_ASSERT_EQUAL(index, set.index_of(found_iter));

        const size_t found_index2 = set.index_of(i);
        AWL_ASSERT_EQUAL(index, found_index2);

        ++index;
    }

    for (size_t i = 0; i < 5; ++i)
    {
        Assert::Throws<std::out_of_range>([&set, i]()
        {
            set.at(set.size() + i);
        });

        Assert::Throws<std::out_of_range>([&set, range, i]()
        {
            set.index_of(range + 1 + i);
        });
    }
}

namespace
{
    struct A
    {
        A() = default;

        explicit A(size_t k) : key(k), attribute(k + 1)
        {
        }

        size_t key;
        size_t attribute;

        size_t GetKey() const
        {
            return key;
        }

        const size_t & GetKeyRef() const
        {
            return key;
        }

        //for testing
        AWL_TUPLIZABLE(key)
    };

    //for testing
    AWL_MEMBERWISE_EQUATABLE(A)

    template <class Compare>
    void TestComparer(const TestContext& context, Compare comp = {})
    {
        AWL_ATTRIBUTE(size_t, insert_count, 1000u);
        AWL_ATTRIBUTE(size_t, range, 1000u);

        auto set = GenerateIntSet<size_t, A, Compare>(insert_count, range, comp);

        size_t index = 0;

        for (auto val : set)
        {
            const auto found_val = set.at(index);
            AWL_ASSERT(val == found_val);

            const auto i = set.find(val.key);
            AWL_ASSERT(i != set.end() && *i == val);

            const size_t found_index = set.index_of(val);
            AWL_ASSERT_EQUAL(index, found_index);

            ++index;
        }

        for (size_t i = 0; i < 5; ++i)
        {
            Assert::Throws<std::out_of_range>([&set, i]()
            {
                set.at(set.size() + i);
            });

            Assert::Throws<std::out_of_range>([&set, range, i]()
            {
                set.index_of(range + 1 + i);
            });
        }
    }

    template <class Compare>
    void TestPointerComparer(const TestContext & context, Compare comp)
    {
        AWL_ATTRIBUTE(size_t, insert_count, 1000u);
        AWL_ATTRIBUTE(size_t, range, 1000u);

        std::vector<A> v;

        std::uniform_int_distribution<size_t> dist(1u, range);

        for (size_t i = 0; i < insert_count; ++i)
        {
            A val(dist(awl::random()));
            v.push_back(val);
        }

        std::sort(v.begin(), v.end(), awl::member_compare<&A::key>());
        v.erase(std::unique(v.begin(), v.end()), v.end());

        awl::vector_set<const A *, Compare> set(comp);

        for (const A & val : v)
        {
            set.insert(&val);
        }

        for (size_t index = 0; index < v.size(); ++index)
        {
            const A & val = v[index];
            const A * found_val = set.at(index);
            AWL_ASSERT(val == *found_val);

            AWL_ASSERT_EQUAL(index, set.index_of(&val));
            AWL_ASSERT_EQUAL(index, set.index_of(val.key));

            {
                auto i = set.find(&val);
                AWL_ASSERT(i != set.end());
                AWL_ASSERT(*(*i) == val);
            }

            {
                auto i = set.find(val.key);
                AWL_ASSERT(i != set.end());
                AWL_ASSERT(*(*i) == val);
            }
        }
    }

    template <class Compare>
    void TestSmartPointerComparer(const TestContext & context, Compare comp)
    {
        AWL_ATTRIBUTE(size_t, insert_count, 1000u);
        AWL_ATTRIBUTE(size_t, range, 1000u);

        using Pointer = typename Compare::value_type;
        using A = typename Pointer::element_type;

        std::vector<A> v;
        awl::vector_set<Pointer, Compare> set(comp);

        std::uniform_int_distribution<size_t> dist(1u, range);

        for (size_t i = 0; i < insert_count; ++i)
        {
            A val(dist(awl::random()));
            v.push_back(val);
            set.insert(Pointer(new A(val)));
        }

        std::sort(v.begin(), v.end(), awl::member_compare<&A::key>());
        v.erase(std::unique(v.begin(), v.end()), v.end());

        for (size_t index = 0; index < v.size(); ++index)
        {
            const A & val = v[index];
            const Pointer & p_found = set.at(index);
            AWL_ASSERT(val == *p_found);

            AWL_ASSERT_EQUAL(index, set.index_of(Pointer(new A(val))));
            AWL_ASSERT_EQUAL(index, set.index_of(val.key));

            {
                auto i = set.find(Pointer(new A(val)));
                AWL_ASSERT(i != set.end());
                AWL_ASSERT(*(*i) == val);
            }

            {
                auto i = set.find(val.key);
                AWL_ASSERT(i != set.end());
                AWL_ASSERT(*(*i) == val);
            }
        }
    }
}

AWL_TEST(VectorSetComparer)
{
    TestComparer(context, awl::member_compare<&A::key>());
    TestComparer(context, awl::member_compare<&A::GetKey>());
    TestComparer(context, awl::member_compare<&A::GetKeyRef>());
    TestComparer(context, awl::tuplizable_compare<A, 0>{});

    TestPointerComparer(context, awl::pointer_compare<&A::key>());
    TestPointerComparer(context, awl::pointer_compare<&A::GetKey>());
    TestPointerComparer(context, awl::pointer_compare<&A::GetKeyRef>());
    TestPointerComparer(context, awl::tuplizable_compare<A*, 0>{});

    TestSmartPointerComparer(context, awl::shared_compare<&A::key>());
    TestSmartPointerComparer(context, awl::shared_compare<&A::GetKey>());
    TestSmartPointerComparer(context, awl::shared_compare<&A::GetKeyRef>());
    TestSmartPointerComparer(context, awl::tuplizable_compare<std::shared_ptr<A>, 0>{});

    TestSmartPointerComparer(context, awl::unique_compare<&A::key>());
    TestSmartPointerComparer(context, awl::unique_compare<&A::GetKey>());
    TestSmartPointerComparer(context, awl::unique_compare<&A::GetKeyRef>());
    TestSmartPointerComparer(context, awl::tuplizable_compare<std::unique_ptr<A>, 0>{});
}

namespace
{
    class B
    {
    public:

        B(int k) : key(k)
        {
        }

        B(const B &) = delete;
        B(B &&) = default;

        int GetKey() const
        {
            return key;
        }

        //for testing
        AWL_TUPLIZABLE(key)

    private:

        int key;
    };

    //for testing
    AWL_MEMBERWISE_EQUATABLE_AND_COMPARABLE(B)

    template <class Compare>
    void TestBComparer(size_t insert_count, int range, Compare comp)
    {
        auto set = GenerateIntSet<int, B, Compare>(insert_count, range, comp);

        size_t index = 0;

        for (auto & val : set)
        {
            const auto & found_val = set.at(index);
            AWL_ASSERT(val == found_val);

            const auto i = set.find(val.GetKey());
            AWL_ASSERT(i != set.end() && *i == val);

            const size_t found_index = set.index_of(val);
            AWL_ASSERT_EQUAL(index, found_index);

            ++index;
        }

        for (size_t i = 0; i < 5; ++i)
        {
            Assert::Throws<std::out_of_range>([&set, i]()
            {
                set.at(set.size() + i);
            });

            Assert::Throws<std::out_of_range>([&set, range, i]()
            {
                set.index_of(static_cast<int>(range + 1 + i));
            });
        }
    }
}

AWL_TEST(VectorSetNonCopyableElement)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(int, range, 2000);

    auto set = GenerateIntSet<int, B>(insert_count, range);

    {
        std::uniform_int_distribution<int> dist(1, range);

        for (size_t i = 0; i < insert_count; ++i)
        {
            const int val = dist(awl::random());
            set.insert(B(val));
        }
    }

    std::vector<int> keys;
    for (const B & b : set)
    {
        keys.push_back(b.GetKey());
    }

    awl::vector_set<B> set1 = std::move(set);

    for (size_t i = 0; i < keys.size(); ++i)
    {
        const int found_key = set1.at(i).GetKey();
        const int actual_key = keys[i];
        AWL_ASSERT_EQUAL(found_key, actual_key);

        AWL_ASSERT_EQUAL(set1.index_of(found_key), i);
        AWL_ASSERT_EQUAL(set1.index_of(B(found_key)), i);
        AWL_ASSERT(set1.find(B(found_key)) != set1.end());
    }

    TestBComparer(insert_count, range, awl::make_compare(&B::GetKey));
    TestBComparer(insert_count, range, awl::tuplizable_compare<B, 0>{});
}

template <class I1, class I2>
static void AssertIteratorsEqual(const I1 & i1, const I1 & end1, const I2 & i2, const I2 & end2)
{
    if (!(i1 == end1 && i2 == end2))
    {
        if (i1 != end1 && i2 != end2)
        {
            const size_t val1 = *i1;
            const size_t val2 = *i2;

            AWL_ASSERTM_EQUAL(val1, val2, _T("different values"));
        }
        else
        {
            AWL_FAILM(_T("end and not end"));
        }
    }
}

template <class Set1, class Set2>
static void TestBound(Set1 & set, Set2 & std_set, size_t range, size_t iter_count)
{
    std::uniform_int_distribution<size_t> dist(0, range * 2);

    for (size_t i = 0; i < iter_count; ++i)
    {
        const size_t val = dist(awl::random());

        AssertIteratorsEqual(set.lower_bound(val), set.end(), std_set.lower_bound(val), std_set.end());
        AssertIteratorsEqual(set.upper_bound(val), set.end(), std_set.upper_bound(val), std_set.end());
        //since C++20
        AWL_ASSERT_EQUAL(set.contains(val), std_set.find(val) != std_set.end());
    }
}

//--filter VectorSetBoundAndContains.* --insert_count 1000 --range 1200
AWL_TEST(VectorSetBoundAndContains)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(size_t, range, 1000);
    AWL_ATTRIBUTE(size_t, iter_count, 1000);

    auto set = GenerateIntSet(insert_count, range);

    std::set<size_t> std_set;

    for (auto val : set)
    {
        std_set.insert(val);
    }

    TestBound(set, std_set, range, iter_count);

    const auto & const_set = set;
    const auto & const_std_set = std_set;

    TestBound(const_set, const_std_set, range, iter_count);
}

AWL_TEST(VectorSetBidirectional)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(size_t, range, 1000);

    auto set = GenerateIntSet(insert_count, range);

    auto comp = set.value_comp();

    for (auto i = set.begin(); i != set.end(); ++i)
    {
        AWL_ASSERT(set.lower_bound(*i) == i);

        if (i != set.begin())
        {
            auto i_prev = i;
            --i_prev;

            AWL_ASSERT(set.upper_bound(*i_prev) == i);

            AWL_ASSERT(comp(*i_prev, *i));
        }

        auto i_next = i;
        ++i_next;

        AWL_ASSERT(set.upper_bound(*i) == i_next);

        if (i_next != set.end())
        {
            AWL_ASSERT(comp(*i, *i_next));
        }
    }

    AWL_ASSERT(set.upper_bound(*(--set.end())) == set.end());
}
