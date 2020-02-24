#include "Awl/ObservableSet.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Random.h"
#include "Awl/String.h"
#include "Awl/KeyCompare.h"
#include "Awl/Serializable.h"

using namespace awl::testing;

namespace
{
    struct A
    {
        A() = default;

        explicit A(size_t k) : key(k), attribute(k + 1)
        {
        }

        size_t key;
        size_t attribute;

        size_t GetKey() const
        {
            return key;
        }

        const size_t & GetKeyRef() const
        {
            return key;
        }

        //for testing
        AWL_SERIALIZABLE(key)
    };
}

AWT_TEST(ObservableSetAssignment)
{
    AWT_UNUSED_CONTEXT;

    using Compare = awl::FieldCompare<A, size_t, &A::key>;
    using Set = awl::observable_set<A, Compare>;

    Set s;
    s = {};
    s.insert(A(1));
    s = {};
    s.insert(A(1));
}
