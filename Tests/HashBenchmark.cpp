
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>

#include "Awl/Hash.h"
#include "Awl/String.h"

#include "Awl/StopWatch.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

static double ReportSpeed(const TestContext & context, const awl::StopWatch & w, size_t size)
{
    const auto time = w.GetElapsedSeconds<double>();

    const double speed = size / time / (1024 * 1024);
    
    context.out << std::fixed << std::setprecision(2) << speed << _T(" MB/sec");

    return speed;
}

template <class Hash>
static void CalcHash(const TestContext & context)
{
    AWL_ATTRIBUTE(size_t, vector_size, 1000000);
    AWL_ATTRIBUTE(size_t, iteration_count, 1);

    std::unique_ptr<uint8_t[]> p_buffer(new uint8_t[vector_size]);
    
    for (int i = 0; i < vector_size; ++i)
    {
        p_buffer[i] = static_cast<uint8_t>(i);
    }

    Hash hash;

    Hash::value_type val = 0;

    {
        awl::StopWatch w;

        for (int i = 0; i < iteration_count; ++i)
        {
            Hash::value_type new_val = hash(p_buffer.get(), vector_size);

            if (val == 0)
            {
                val = new_val;
            }
            else
            {
                Assert::IsTrue(new_val == val);
            }
        }

        context.out << awl::FromACString(typeid(hash).name()) << _T(": ");

        ReportSpeed(context, w, vector_size * iteration_count * sizeof(uint8_t));

        context.out << _T(" Hash : ") << val << _T(" ") << std::endl;
    }
}

AWL_BENCHMARK(HashPerformance)
{
    using namespace awl::crypto;
    
    CalcHash<Crc64>(context);
}
