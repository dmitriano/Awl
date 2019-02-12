#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>

#include "Awl/StopWatch.h"
#include "Awl/IntRange.h"
#include "Awl/Testing/UnitTest.h"

#include "BenchmarkHelpers.h"

using namespace awl::testing;

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

private:

    int i;
    double x;
    long double y;
};


template <class T>
static void FromInt(T & val, int i)
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
static void CopyVector(const TestContext & context, const awl::Char * type_name)
{
    AWL_ATTRIBUTE(size_t, vector_size, 1000000);
    AWL_ATTRIBUTE(size_t, iteration_count, 100);

    std::unique_ptr<T[]> p_buffer(new T[vector_size]);
    
    for (auto i : awl::make_count(static_cast<int>(vector_size)))
    {
        FromInt(p_buffer[i], i);
    }

    std::vector<T> v;
    v.reserve(vector_size);
    Assert::AreEqual(vector_size, v.capacity());

    context.out << _T("std::vector<") << type_name << _T(">\t");

    double ratio;
    
    {
        awl::StopWatch w;

        for (auto i : awl::make_count(iteration_count))
        {
            static_cast<void>(i);

            std::copy(p_buffer.get(), p_buffer.get() + vector_size, std::back_inserter(v));

            //Ensure the vector was not resized.
            Assert::AreEqual(vector_size, v.capacity());
            Assert::AreEqual(vector_size, v.size());
            v.resize(0);
        }

        context.out << _T("copy: ");

        ratio = ReportSpeed(context, w, vector_size * iteration_count * sizeof(T));
    }

    {
        awl::StopWatch w;

        for (auto i : awl::make_count(iteration_count))
        {
            static_cast<void>(i);

            v.insert(v.end(), p_buffer.get(), p_buffer.get() + vector_size);

            //Ensure the vector was not resized.
            Assert::AreEqual(vector_size, v.capacity());
            Assert::AreEqual(vector_size, v.size());
            v.resize(0);
        }

        context.out << _T("\tinsert: ");

        ratio = ReportSpeed(context, w, vector_size * iteration_count * sizeof(T)) / ratio;
    }

    context.out << _T("\t (") << ratio << _T(")");

    context.out << _T("\tsizeof(") << type_name << _T("): ") << sizeof(T) << _T("\t") << std::endl;
}

AWT_BENCHMARK(VectorCopyPerformance)
{
    CopyVector<uint8_t>(context, _T("byte"));
    CopyVector<short>(context, _T("short"));
    CopyVector<int>(context, _T("int"));
    CopyVector<long>(context, _T("long"));
    CopyVector<long long>(context, _T("long long"));
    CopyVector<float>(context, _T("float"));
    CopyVector<double>(context, _T("double"));
    CopyVector<long double>(context, _T("long double"));
    CopyVector<long long>(context, _T("long double"));
    CopyVector<ElementStruct>(context, _T("struct"));
    CopyVector<ElementClass>(context, _T("class"));
}
