/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/ForeignSet.h"
#include "Awl/Random.h"
#include "Awl/KeyCompare.h"
#include "Awl/Tuplizable.h"

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

        AWL_TUPLIZABLE(pk, fk)
    };
    
    AWL_MEMBERWISE_EQUATABLE(A)

    using PrimaryGetter = awl::field_getter<A, int>; //&A::pk
    using ForeignGetter = awl::field_getter<A, int>; //&A::fk

    using PrimarySet = awl::observable_set<A, awl::KeyCompare<A, PrimaryGetter>>;
    using ForeignSet = awl::foreign_set<A, PrimaryGetter, ForeignGetter>;

    //Do we really need it to be const?
    static_assert(std::is_same_v<typename ForeignSet::value_type::value_type, const A*>);

    static void GenerateSet(PrimarySet & sample, size_t insert_count, int range)
    {
        std::uniform_int_distribution<int> dist(1, range);

        for (size_t i = 0; i <insert_count; ++i)
        {
            sample.insert(A{ dist(awl::random()) , dist(awl::random()) });
        }
    }

    static void ResampleSet(PrimarySet & sample, size_t insert_count, int range)
    {
        std::uniform_int_distribution<int> dist(1, range);

        for (int val = 0; val < static_cast<int>(insert_count); ++val)
        {
            auto i = sample.find(val);

            if (i != sample.end())
            {
                sample.erase(i);
            }
        }
    }
}

AWL_TEST(ForeignSetAddRemoveClear)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(int, range, 1000);

    PrimarySet ps{ PrimaryGetter{ &A::pk } };
    ForeignSet fs{ PrimaryGetter{ &A::pk }, ForeignGetter{ &A::fk } };

    auto check = [&]()
    {
        for (int val = 1; val <= range; ++val)
        {
            const size_t f_count = std::count_if(ps.begin(), ps.end(), [val](const A & a) { return a.fk == val; });

            auto i = fs.find(val);

            if (f_count != 0)
            {
                AWL_ASSERT(i != fs.end());

                const auto & vs = *i;
                
                AWL_ASSERT_EQUAL(f_count, vs.size());
            }
            else
            {
                AWL_ASSERT(i == fs.end());
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

    AWL_ASSERT(fs.empty());
}

AWL_TEST(ForeignSetDestructor)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(int, range, 1000);

    ForeignSet fs{ PrimaryGetter{ &A::pk }, ForeignGetter{ &A::fk } };
    
    {
        PrimarySet ps{ PrimaryGetter{ &A::pk } };

        ps.Subscribe(&fs);

        GenerateSet(ps, insert_count, range);
    }

    AWL_ASSERT(fs.empty());
}

AWL_TEST(ForeignSetConstructor)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(int, range, 1000);

    PrimarySet ps{ PrimaryGetter{ &A::pk } };

    GenerateSet(ps, insert_count / 2, range / 2);

    ForeignSet fs(ps, PrimaryGetter{ &A::pk }, ForeignGetter{ &A::fk });

    GenerateSet(ps, insert_count / 2, range / 2);

    size_t count = 0;
    
    for (auto& set : fs)
    {
        count += set.size();
    }

    AWL_ASSERT_EQUAL(ps.size(), count);
}

AWL_TEST(ForeignSetShared)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(int, range, 1000);

    {
        A a;
        std::shared_ptr<A> shared_p = std::make_shared<A>(A{ 1, 2 });
        std::shared_ptr<A>& p = shared_p;

        static_assert(awl::is_pointer_v<std::decay_t<decltype(p)>>);

        static_assert(std::is_same_v<decltype(*p), A&>);

        auto plain_p = awl::object_address(p);
        static_assert(std::is_same_v<decltype(plain_p), A*>);

        auto p_a = awl::object_address(a);
        static_assert(std::is_same_v<decltype(p_a), A*>);

        ForeignGetter foreign_getter{ &A::fk };
        const int key = foreign_getter(*plain_p);
        static_cast<void>(key);
    }

    //Check if it compiles.
    
    using SharedPrimarySet = awl::observable_set<std::shared_ptr<A>, awl::KeyCompare<std::shared_ptr<A>, PrimaryGetter>>;
    using SharedForeignSet = awl::foreign_set<std::shared_ptr<A>, PrimaryGetter, ForeignGetter>;

    static_assert(std::is_same_v<typename SharedForeignSet::value_type::value_type, std::shared_ptr<A>>);

    SharedForeignSet fs{ PrimaryGetter{&A::pk}, ForeignGetter{&A::fk} };

    {
        SharedPrimarySet ps{ PrimaryGetter{&A::pk} };

        ps.Subscribe(&fs);

        std::uniform_int_distribution<int> dist(1, range);

        for (size_t i = 0; i < insert_count; ++i)
        {
            ps.insert(std::make_shared<A>(A{ dist(awl::random()) , dist(awl::random()) }));
        }
    }

    AWL_ASSERT(fs.empty());
}

AWL_TEST(ForeignSetUnique)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(int, range, 1000);

    //Check if it compiles.

    using UniquePrimarySet = awl::observable_set<std::unique_ptr<A>, awl::KeyCompare<std::unique_ptr<A>, PrimaryGetter>>;
    using UniqueForeignSet = awl::foreign_set<std::unique_ptr<A>, PrimaryGetter, ForeignGetter>;

    static_assert(std::is_same_v<typename UniqueForeignSet::value_type::value_type, const A*>);

    UniqueForeignSet fs{ PrimaryGetter{&A::pk}, ForeignGetter{&A::fk} };
    
    {
        UniquePrimarySet ps{ PrimaryGetter{&A::pk} };

        ps.Subscribe(&fs);

        std::uniform_int_distribution<int> dist(1, range);

        for (size_t i = 0; i < insert_count; ++i)
        {
            ps.insert(std::make_unique<A>(A{ dist(awl::random()) , dist(awl::random()) }));
        }
    }

    AWL_ASSERT(fs.empty());
}

AWL_TEST(ForeignSetPlainPointer)
{
    AWL_ATTRIBUTE(size_t, insert_count, 1000);
    AWL_ATTRIBUTE(int, range, 1000);

    //Check if it compiles.

    using PointerPrimarySet = awl::observable_set<A *, awl::KeyCompare<A *, PrimaryGetter>>;
    using PointerForeignSet = awl::foreign_set<A *, PrimaryGetter, ForeignGetter>;

    static_assert(std::is_same_v<typename PointerForeignSet::value_type::value_type, A*>);

    PointerForeignSet fs{ PrimaryGetter{&A::pk}, ForeignGetter{&A::fk} };
    PointerPrimarySet ps{ PrimaryGetter{&A::pk} };

    ps.Subscribe(&fs);

    std::uniform_int_distribution<int> dist(1, range);

    for (size_t i = 0; i < insert_count; ++i)
    {
        A* p = new A{ dist(awl::random()) , dist(awl::random()) };
        
        if (!ps.insert(p).second)
        {
            delete p;
        }
    }

    while (!ps.empty())
    {
        A* p = ps.front();
        
        ps.erase(p);

        delete p;
    }

    AWL_ASSERT(fs.empty());
}
