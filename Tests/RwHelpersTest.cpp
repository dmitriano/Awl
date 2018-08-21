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

    Assert::AreEqual(sizeof(sample), WriteScalar(out, sample));

    VectorInputStream in(v);

    size_t result;

    Assert::AreEqual(sizeof(sample), ReadScalar(in, result));

    Assert::AreEqual(sample, result);

    Assert::AreEqual(0, in.Read(nullptr, 10));
}
