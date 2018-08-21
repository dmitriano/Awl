#include "Awl/Io/RwHelpers.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Testing/UnitTest.h"

#include <iostream>
#include <algorithm>
#include <functional>

using namespace awl::testing;
using namespace awl::io;

AWL_TEST(IoScalarReadWrite)
{
    std::hash<int> hasher;

    size_t sample = hasher(0);

    std::vector<uint8_t> v;

    VectorOutputStream out(v);

    WriteScalar(out, sample);

    VectorInputStream in(v);

    size_t result;

    ReadScalar(in, result);

    Assert::AreEqual(sample, result);

    Assert::IsTrue(in.End());
}
