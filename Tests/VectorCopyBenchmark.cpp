#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>

#include "Awl/StopWatch.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StdConsole.h"

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
static void FromInt(ElementStruct & val, int i)
{
    val.i = i;
    val.x = i * 10.0;
    val.y = i * 10.0;
}

static double ReportSpeed(const TestContext & context, const awl::StopWatch & w, size_t size)
{
    const auto time = w.GetElapsedSeconds<double>();

    const double speed = size / time / (1024 * 1024);
    
    context.out << std::fixed << std::setprecision(2) << speed << _T(" MB/sec");

    return speed;
}

template <class T>
static void CopyVector(const TestContext & context, const awl::Char * type_name)
{
    AWL_ATTRIBUTE(size_t, vector_size, 1000000);
    AWL_ATTRIBUTE(size_t, iteration_count, 1);

    std::unique_ptr<T[]> p_buffer(new T[vector_size]);
    
    for (int i = 0; i < vector_size; ++i)
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

        for (int i = 0; i < iteration_count; ++i)
        {
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

        for (int i = 0; i < iteration_count; ++i)
        {
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

AWL_BENCHMARK(VectorCopyPerformance)
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