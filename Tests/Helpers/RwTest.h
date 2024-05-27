/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/TypeTraits.h"
#include "Awl/Io/ReadWrite.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;
using namespace awl::io;

namespace awl::testing::helpers
{
    template <class T>
    void TestReadWrite(const TestContext& context, const T& sample)
    {
        AWT_ATTRIBUTE(size_t, iteration_count, 10);

        std::vector<uint8_t> reusable_v;

        VectorOutputStream out(reusable_v);

        for (size_t i = 0; i < iteration_count; ++i)
        {
            Write(out, sample);
        }

        VectorInputStream in(reusable_v);

        for (size_t i = 0; i < iteration_count; ++i)
        {
            T result;

            Read(in, result);

            if constexpr (is_pointer_v<T>)
            {
                AWT_ASSERT((sample == nullptr && result == nullptr) ||
                    (sample != nullptr && result != nullptr && *sample == *result));
            }
            else
            {
                AWT_ASSERT(sample == result);
            }
        }

        AWT_ASSERT(in.End());
    }
}
