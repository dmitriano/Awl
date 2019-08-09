#include "Awl/ForeignSet.h"
#include "Awl/Random.h"
#include "Awl/KeyCompare.h"
#include "Awl/Serializable.h"

#include "Awl/Testing/UnitTest.h"

#include <algorithm>
#include <set>

using namespace awl::testing;

namespace
{
    struct A
    {
        int pk;
        int fk;

        AWL_SERIALIZABLE(pk, fk)
    };
    
    AWL_MEMBERWISE_EQUATABLE(A)

    using PrimaryGetter = awl::FieldGetter<A, int, &A::pk>;
    using ForeignGetter = awl::FieldGetter<A, int, &A::fk>;

    using PrimarySet = awl::observable_set<A, awl::KeyCompare<A, PrimaryGetter>>;
    using ForeignSet = awl::foreign_set<ForeignGetter, A, PrimaryGetter>;

    static void GenerateSet(PrimarySet & sample, size_t insert_count, int range)
    {
        std::uniform_int_distribution<int> dist(1, range);

        for (size_t i = 0; i < insert_count; ++i)
        {
            sample.insert(A{ dist(awl::random()) , dist(awl::random()) });
        }
    }

    static void ResampleSet(PrimarySet & sample, size_t insert_count, int range)
    {
        std::uniform_int_distribution<int> dist(1, range);

        for (int val = 0; val < insert_count; ++val)
        {
            auto i = sample.find(val);

            if (i != sample.end())
            {
                sample.erase(i);
            }
        }
    }
}

AWT_TEST(ForeignSet)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(int, range, 1000);

    PrimarySet ps;
    ForeignSet fs;

    auto check = [&]()
    {
        for (int val = 1; val <= range; ++val)
        {
            const size_t f_count = std::count_if(ps.begin(), ps.end(), [val](const A & a) { return a.fk == val; });

            auto i = fs.find(val);

            if (f_count != 0)
            {
                Assert::IsTrue(i != fs.end());

                const auto & vs = *i;
                
                Assert::AreEqual(f_count, vs.size());
            }
            else
            {
                Assert::IsTrue(i == fs.end());
            }
        }
    };

    ps.Subscribe(&fs);

    GenerateSet(ps, insert_count, range);

    check();

    ResampleSet(ps, insert_count, range);

    check();

    ps.clear();

    check();

    Assert::IsTrue(fs.empty());
}
