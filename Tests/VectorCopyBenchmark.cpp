/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <future>
#include <ranges>
#include <span>
#include <numeric>

#include "Awl/StopWatch.h"
#include "Awl/IntRange.h"
#include "Awl/Testing/UnitTest.h"

#include "Helpers/BenchmarkHelpers.h"

using namespace awl::testing;

namespace
{
    struct ElementStruct
    {
        int i;
        double x;
        long double y;
    };

    class ElementClass
    {
    public:

        ElementClass() : i(0), x(12.0), y(77.0)
        {
        }

        ElementClass(int val) : i(val), x(val * 12.0), y(val * 77.0)
        {
        }

        double ToReal() const
        {
            return static_cast<double>(i + x + y);
        }

    private:

        int i;
        double x;
        long double y;
    };

    template <class T>
    void FromInt(T & val, int i)
    {
        val = static_cast<T>(i);
    }

    template <>
    void FromInt(ElementStruct & val, int i)
    {
        val.i = i;
        val.x = i * 10.0;
        val.y = i * 10.0;
    }

    template <class T>
    double ToReal(const T& val)
    {
        return static_cast<double>(val);
    }

    template <>
    double ToReal(const ElementStruct& val)
    {
        return static_cast<double>(val.i + val.x + val.y);
    }

    template <>
    double ToReal(const ElementClass& val)
    {
        return val.ToReal();
    }

    template <class T>
    struct CopyVector
    {
        void operator()(const TestContext& context, const awl::Char* type_name)
        {
            AWL_ATTRIBUTE(size_t, vector_size, 1000000);
            AWL_ATTRIBUTE(size_t, iteration_count, 1);

            std::unique_ptr<T[]> p_buffer(new T[vector_size]);

            for (auto i : awl::make_count(static_cast<int>(vector_size)))
            {
                FromInt(p_buffer[i], i);
            }

            std::vector<T> v;
            v.reserve(vector_size);
            AWL_ASSERT_EQUAL(vector_size, v.capacity());

            context.out << _T("std::vector<") << type_name << _T(">\t");

            double ratio;

            {
                awl::StopWatch w;

                for (auto i : awl::make_count(iteration_count))
                {
                    static_cast<void>(i);

                    std::copy(p_buffer.get(), p_buffer.get() + vector_size, std::back_inserter(v));

                    //Ensure the vector was not resized.
                    AWL_ASSERT_EQUAL(vector_size, v.capacity());
                    AWL_ASSERT_EQUAL(vector_size, v.size());
                    v.resize(0);
                }

                context.out << _T("copy: ");

                ratio = helpers::ReportSpeed(context, w, vector_size * iteration_count * sizeof(T));
            }

            {
                awl::StopWatch w;

                for (auto i : awl::make_count(iteration_count))
                {
                    static_cast<void>(i);

                    v.insert(v.end(), p_buffer.get(), p_buffer.get() + vector_size);

                    //Ensure the vector was not resized.
                    AWL_ASSERT_EQUAL(vector_size, v.capacity());
                    AWL_ASSERT_EQUAL(vector_size, v.size());
                    v.resize(0);
                }

                context.out << _T("\tinsert: ");

                ratio = helpers::ReportSpeed(context, w, vector_size * iteration_count * sizeof(T)) / ratio;
            }

            context.out << _T("\t (") << ratio << _T(")");

            context.out << _T("\tsizeof(") << type_name << _T("): ") << sizeof(T) << _T("\t") << std::endl;
        }
    };

    template <class T>
    struct CopyVectorAsync
    {
        void operator()(const TestContext& context, const awl::Char* type_name)
        {
            AWL_ATTRIBUTE(size_t, vector_size, 1000000);
            AWL_ATTRIBUTE(size_t, iteration_count, 1);

            context.out << _T("std::vector<") << type_name << _T(">\t");

            std::unique_ptr<T[]> p_buffer;

            std::vector<T> v;
            
            try
            {
                p_buffer = std::make_unique<T[]>(vector_size);

                v.resize(vector_size);
            }
            catch (const std::bad_alloc&)
            {
                context.out << "Too long vector. Can't allocate memory." << std::endl;

                return;
            }

            for (auto i : awl::make_count(static_cast<int>(vector_size)))
            {
                FromInt(p_buffer[i], i);
            }

            AWL_ASSERT_EQUAL(vector_size, v.size());

            auto copy = [&](size_t begin, size_t end)
            {
                for (auto i : awl::make_count(iteration_count))
                {
                    static_cast<void>(i);

                    std::copy(p_buffer.get() + begin, p_buffer.get() + end, v.begin() + begin);
                }
            };

            double ratio;

            {
                awl::StopWatch w;

                std::vector<std::future<void>> futures;

                const size_t thread_count = std::thread::hardware_concurrency();

                const size_t chunk_size = vector_size / thread_count;
                
                for (size_t i = 0; i < thread_count; ++i)
                {
                    const size_t begin = i * chunk_size;

                    const size_t end = begin + chunk_size;

                    futures.push_back(std::async(std::launch::async, copy, begin, end));
                }

                futures.push_back(std::async(std::launch::async, copy, chunk_size * thread_count, vector_size));

                std::for_each(futures.begin(), futures.end(), [](std::future<void>& f) { f.get(); });

                context.out << _T("async: ");

                ratio = helpers::ReportSpeed(context, w, vector_size * iteration_count * sizeof(T));
            }

            {
                awl::StopWatch w;

                copy(0, vector_size);

                context.out << _T("\tsync: ");

                ratio = helpers::ReportSpeed(context, w, vector_size * iteration_count * sizeof(T)) / ratio;
            }

            context.out << _T("\t (") << ratio << _T(")");

            context.out << _T("\tsizeof(") << type_name << _T("): ") << sizeof(T) << _T("\t") << std::endl;
        }
    };

    template <class T>
    struct SumVector
    {
        void operator()(const TestContext& context, const awl::Char* type_name)
        {
            AWL_ATTRIBUTE(size_t, vector_size, 1000000);
            AWL_ATTRIBUTE(size_t, iteration_count, 1);
            AWL_ATTRIBUTE(size_t, thread_count, std::thread::hardware_concurrency());
            AWL_FLAG(show_result);

            context.out << _T("std::vector<") << type_name << _T(">\t");

            std::unique_ptr<T[]> p_buffer;

            try
            {
                p_buffer = std::make_unique<T[]>(vector_size);
            }
            catch (const std::bad_alloc&)
            {
                context.out << "Too long vector. Can't allocate memory." << std::endl;

                return;
            }

            for (auto i : awl::make_count(static_cast<int>(vector_size)))
            {
                FromInt(p_buffer[i], i);
            }

            auto sum = [&](size_t begin, size_t end) -> double
            {
                double result = 0.0;
                
                for (auto i : awl::make_count(iteration_count))
                {
                    static_cast<void>(i);

                    std::span<T> span(p_buffer.get() + begin, p_buffer.get() + end);

                    auto range = span | std::views::transform([](const T& val) -> double { return ToReal(val); });

                    result += std::accumulate(range.begin(), range.end(), 0.0);
                }

                return result;
            };

            double ratio;

            {
                awl::StopWatch w;

                std::vector<std::future<double>> futures;

                const size_t chunk_size = vector_size / thread_count;

                for (size_t i = 0; i < thread_count; ++i)
                {
                    const size_t begin = i * chunk_size;

                    const size_t end = begin + chunk_size;

                    futures.push_back(std::async(std::launch::async, sum, begin, end));
                }

                futures.push_back(std::async(std::launch::async, sum, chunk_size * thread_count, vector_size));

                auto range = futures | std::views::transform([](std::future<double>& f) -> double { return f.get(); });
                
                const double result = std::accumulate(range.begin(), range.end(), 0.0);

                context.out << _T("\tasync: ");

                if (show_result)
                {
                    context.out << _T("\tresult=" << result << ", ");
                }

                ratio = helpers::ReportSpeed(context, w, vector_size * iteration_count * sizeof(T));
            }

            {
                awl::StopWatch w;

                const double result = sum(0, vector_size);

                context.out << _T("\tsync: ");

                if (show_result)
                {
                    context.out << _T("\tresult=" << result << ", ");
                }

                ratio = helpers::ReportSpeed(context, w, vector_size * iteration_count * sizeof(T)) / ratio;
            }

            context.out << _T("\t (") << ratio << _T(")");

            context.out << _T("\tsizeof(") << type_name << _T("): ") << sizeof(T) << _T("\t") << std::endl;
        }
    };

    template <template <class> class copy>
    void CopyVectors(const TestContext& context)
    {
        copy<uint8_t>{}(context, _T("byte"));
        copy<short>{}(context, _T("short"));
        copy<int>{}(context, _T("int"));
        copy<long>{}(context, _T("long"));
        copy<long long>{}(context, _T("long long"));
        copy<float>{}(context, _T("float"));
        copy<double>{}(context, _T("double"));
        copy<long double>{}(context, _T("long double"));
        copy<long long>{}(context, _T("long long"));
        copy<ElementStruct>{}(context, _T("struct"));
        copy<ElementClass>{}(context, _T("class"));
    }
}

AWL_BENCHMARK(VectorCopy)
{
    CopyVectors<CopyVector>(context);
}

AWL_BENCHMARK(VectorCopyAsync)
{
    context.out << _T("hardware concurrency: ") << std::thread::hardware_concurrency() << std::endl;

    CopyVectors<CopyVectorAsync>(context);
}

AWL_BENCHMARK(VectorSum)
{
    context.out << _T("hardware concurrency: ") << std::thread::hardware_concurrency() << std::endl;

    CopyVectors<SumVector>(context);
}
