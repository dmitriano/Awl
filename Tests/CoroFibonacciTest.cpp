/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Coro/Generator.h"

#include "Awl/Testing/UnitTest.h"

#include <coroutine>
#include <exception>
#include <limits>
#include <tuple>

namespace
{
    awl::generator<std::tuple<uint64_t, uint64_t>> fibonacci(uint64_t count)
    {
        if (count > 0)
        {
            uint64_t j = 0;
            uint64_t i = 1;

            co_yield std::make_tuple(static_cast<uint64_t>(0u), j);

            for (uint64_t n = 1; n < count; ++n)
            {
                co_yield std::make_tuple(n, i);

                const uint64_t remained = std::numeric_limits<uint64_t>::max() - i;

                if (remained < j)
                {
                    throw std::runtime_error("Too big Fibonacci sequence. Elements would overflow.");
                }
                
                const uint64_t tmp = i;
                i += j;
                j = tmp;
            }
        }
    }

    void PrintFibonacci(const awl::testing::TestContext& context, uint64_t count)
    {
        auto gen = fibonacci(count); //max 94 before uint64_t overflows

        for (auto [n, j] : gen)
        {
            context.out << "fib(" << n << ")=" << j << std::endl;
        }
    }
}

AWT_EXAMPLE(CoroGeneratorFibonacci)
{
    context.out << "Short Fibonacci sequence: " << std::endl;
    
    PrintFibonacci(context, 5);

    context.out << "Long Fibonacci sequence: " << std::endl;

    try
    {
        PrintFibonacci(context, 100);
    }
    catch (const std::exception& ex)
    {
        context.out << "Exception: " << ex.what() << '\n';
    }
    catch (...)
    {
        context.out << "Unknown exception.\n";
    }
}
