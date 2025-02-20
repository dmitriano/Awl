/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>

#include "Awl/StopWatch.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Random.h"
#include "Awl/KeyCompare.h"

#include "Helpers/BenchmarkHelpers.h"

using namespace awl::testing;

namespace
{
    template <class T>
    void Insert(const TestContext& context, const awl::Char* type_name)
    {
        AWL_ATTRIBUTE(size_t, key_range, 1000000);
        AWL_ATTRIBUTE(size_t, value_range, 1000000);
        AWL_ATTRIBUTE(size_t, element_count, 1000);
        AWL_ATTRIBUTE(size_t, iteration_count, 1);

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

        helpers::ReportCount(context, w, element_count * iteration_count);

        context.out << _T("\t") << type_name << std::endl;
    }

    class flat_map
    {
    public:

        using value_type = std::pair<size_t, size_t>;

        using key_compare = std::less<void>;

        using iterator = std::vector<value_type>::iterator;

        std::pair<iterator, bool> insert(const value_type& val)
        {
            auto i = std::lower_bound(m_v.begin(), m_v.end(), val, 
                awl::member_compare<&value_type::first, key_compare>{});

            if (i != m_v.end() && i->first == val.first)
            {
                return { i, false };
            }

            return { m_v.insert(i, val), true };
        }

        std::vector<value_type> m_v;
    };
}

AWL_TEST(FlatMapOrder)
{
    static_cast<void>(context);
    
    flat_map m;

    m.insert(std::make_pair(2, 0));
    m.insert(std::make_pair(20, 3));
    m.insert(std::make_pair(5, 1));
    m.insert(std::make_pair(30, 4));
    m.insert(std::make_pair(10, 2));

    AWL_ASSERT_EQUAL(m.m_v.size(), static_cast<size_t>(5));
    
    for (size_t i = 0; i < m.m_v.size(); ++i)
    {
        AWL_ASSERT_EQUAL(m.m_v[i].second, i);
    }
}

AWL_BENCHMARK(FlatMapInsert)
{
    AWL_FLAG(flat);

    Insert<std::map<size_t, size_t>>(context, _T("map"));
    Insert<std::unordered_map<size_t, size_t>>(context, _T("unordered_map"));

    if (flat)
    {
        Insert<flat_map>(context, _T("vector"));
    }
}

AWL_BENCHMARK(MemoryRead)
{
    AWL_ATTRIBUTE(size_t, element_count, 1024*1024);
    AWL_ATTRIBUTE(size_t, read_count, 1000000);

    using T = uint64_t;

    const size_t size = element_count * sizeof(T);
    
    auto p = std::make_unique<T[]>(size);

    {
        context.out << _T("Init: ");
        
        awl::StopWatch w;

        for (size_t j = 0; j < element_count; ++j)
        {
            p[j] = j;
        }

        helpers::ReportCount(context, w, element_count);

        context.out << std::endl;
    }

    {
        context.out << _T("Write: ");

        awl::StopWatch w;

        std::uniform_int_distribution<size_t> index_dist(0, element_count - 1);

        for (size_t i = 0; i < read_count; ++i)
        {
            size_t index = index_dist(awl::random());

            p[index] = element_count - index;
        }

        helpers::ReportCount(context, w, read_count);

        context.out << std::endl;
    }

    {
        context.out << _T("Read: ");

        awl::StopWatch w;

        std::uniform_int_distribution<size_t> index_dist(0, element_count - 1);

        T sum = 0;

        for (size_t i = 0; i < read_count; ++i)
        {
            size_t index = index_dist(awl::random());

            sum += p[index];
        }

        helpers::ReportCount(context, w, read_count);

        context.out << _T("\t sum=") << sum << std::endl;
    }
}