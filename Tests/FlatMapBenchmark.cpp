#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>

#include "Awl/StopWatch.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Random.h"

#include "BenchmarkHelpers.h"

using namespace awl::testing;

template <class T>
static void Insert(const TestContext & context, const awl::Char * type_name)
{
    AWL_ATTRIBUTE(size_t, key_range, 1000000);
    AWL_ATTRIBUTE(size_t, value_range, 1000000);
    AWL_ATTRIBUTE(size_t, element_count, 100);
    AWL_ATTRIBUTE(size_t, iteration_count, 10000);

    awl::StopWatch w;

    for (size_t i = 0; i < iteration_count; ++i)
    {
        T container;

        std::uniform_int_distribution<size_t> key_dist(0, key_range);
        std::uniform_int_distribution<size_t> value_dist(0, value_range);

        for (size_t j = 0; j < element_count; ++j)
        {
            size_t key = key_dist(awl::random());
            size_t val = value_dist(awl::random());

            //container.emplace(key, val);
            container.insert(std::make_pair(key, val));
        }
    }

    ReportCount(context, w, element_count * iteration_count);

    context.out << _T("\t") << type_name << std::endl;
}

class flat_map
{
public:

    typedef std::pair<size_t, size_t> value_type;

    void insert(const value_type & val)
    {
        auto i = binary_find(m_v.begin(), m_v.end(), val, 
            [&val](const value_type & left, const value_type & right)
            {
                return left.first < right.first;
            });

        if (i != m_v.end())
        {
            ++i;
        }
        
        m_v.insert(i, val);
    }

    std::vector<value_type> m_v;

private:

    template<class ForwardIt, class T, class Compare = std::less<>>
    static ForwardIt binary_find(ForwardIt first, ForwardIt last, const T& value, Compare comp = {})
    {
        // Note: BOTH type T and the type after ForwardIt is dereferenced 
        // must be implicitly convertible to BOTH Type1 and Type2, used in Compare. 
        // This is stricter than lower_bound requirement (see above)

        first = std::lower_bound(first, last, value, comp);
        return first != last && !comp(value, *first) ? first : last;
    }
};

AWT_TEST(FlatMapOrder)
{
    static_cast<void>(context);
    
    flat_map m;

    m.insert(std::make_pair(30, 0));
    m.insert(std::make_pair(20, 1));
    m.insert(std::make_pair(10, 2));
    m.insert(std::make_pair(5, 3));
    m.insert(std::make_pair(2, 4));

    Assert::AreEqual(m.m_v.size(), static_cast<size_t>(5));
    
    for (size_t i = 0; i < m.m_v.size(); ++i)
    {
        Assert::AreEqual(m.m_v[i].second, i);
    }
}

AWT_BENCHMARK(FlatMap)
{
    Insert<std::map<size_t, size_t>>(context, _T("map"));
    Insert<std::unordered_map<size_t, size_t>>(context, _T("unordered_map"));
    Insert<flat_map>(context, _T("vector"));
}
