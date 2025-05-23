/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/BitMap.h"

#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

namespace BitMapTest
{
    AWL_SEQUENTIAL_ENUM(Vehicle, Car, Train, AirPlain)
}

AWL_ENUM_TRAITS(BitMapTest, Vehicle)

namespace BitMapTest
{
    using ConstBitMap = const awl::bitmap<Vehicle>;
    static_assert(awl::EnumTraits<Vehicle>::count() == 3);
    static_assert(ConstBitMap{ Vehicle::Car }[Vehicle::Car]);
    static_assert(!ConstBitMap{ Vehicle::Car }[Vehicle::Train]);
    static_assert(ConstBitMap{ Vehicle::Car, Vehicle::Train }[Vehicle::Train]);

    const std::vector<std::string> expected_names{ "Car", "Train", "AirPlain" };
}

AWL_TEST(BitMapWithEnumTraits)
{
    AWL_UNUSED_CONTEXT;
    
    using namespace BitMapTest;

    AWL_ASSERT(std::ranges::equal(awl::EnumTraits<Vehicle>::names(), expected_names));

    constexpr awl::bitmap<Vehicle> car{ Vehicle::Car };

    AWL_ASSERT(car.count() == 1);
    static_assert(car.size() == 3);

    AWL_ASSERT(car);
    
    static_assert(car[Vehicle::Car]);
    AWL_ASSERT(car & Vehicle::Car);
    AWL_ASSERT(car | Vehicle::Car);
    AWL_ASSERT_FALSE(car ^ Vehicle::Car);

    static_assert(!car[Vehicle::Train]);
    AWL_ASSERT_FALSE(car & Vehicle::Train);
    AWL_ASSERT_FALSE(car & Vehicle::Train);
    AWL_ASSERT(car | Vehicle::Train);
    AWL_ASSERT((car ^ Vehicle::Train) & Vehicle::Car);

    const awl::bitmap<Vehicle> other = ~car;

    AWL_ASSERT_FALSE(other[Vehicle::Car]);
    AWL_ASSERT_FALSE(other & Vehicle::Car);
    AWL_ASSERT(other | Vehicle::Car);
    AWL_ASSERT(other ^ Vehicle::Car);

    AWL_ASSERT(other[Vehicle::Train] && other[Vehicle::AirPlain]);
    AWL_ASSERT((other & Vehicle::Train) && (other & Vehicle::AirPlain));
    AWL_ASSERT_FALSE(other & Vehicle::Car);
    AWL_ASSERT(other | Vehicle::Car);
    AWL_ASSERT_FALSE((other ^ Vehicle::Train) & Vehicle::Car);

    const awl::bitmap<Vehicle> all = car | other;
    AWL_ASSERT(all.all());
    AWL_ASSERT(all == (car ^ other));
    AWL_ASSERT_FALSE(car & other);

    const awl::bitmap<Vehicle> none = ~all;
    AWL_ASSERT(none.none());
    AWL_ASSERT_FALSE(none);

    AWL_ASSERT(all == ~none);

    awl::bitmap<Vehicle> var;
    AWL_ASSERT_FALSE(var);
    var[Vehicle::Car] = true;
    AWL_ASSERT(var[Vehicle::Car]);
    AWL_ASSERT_FALSE(var[Vehicle::Train]);
    AWL_ASSERT_FALSE(var[Vehicle::AirPlain]);
    AWL_ASSERT(var == car);
    AWL_ASSERT(var != other);

    //There is no operator == (bool), but this compiles.
    AWL_ASSERT(var == true);
    AWL_ASSERT(none == false);
}

namespace BitMapTest1
{
    class A
    {
    private:

        AWL_BITMAP(GameLevel1, Baby, Starter, Professional, Expert)

    public:

        AWL_BITMAP(GameLevel, Baby, Starter, Professional, Expert)

        A(GameLevelBitMap bm) : m_bm(bm)
        {
        }

        GameLevelBitMap m_bm;

        static void Test()
        {
            AWL_ASSERT(GameLevel1BitMap{}.none());
            AWL_ASSERT((GameLevel1BitMap{ GameLevel1::Baby, GameLevel1::Starter, GameLevel1::Professional, GameLevel1::Expert }.all()));
        }

        //error: Cannot specialize template in current scope.
        //AWL_ENUM_TRAITS(A, GameLevel)
    };
}

AWL_ENUM_TRAITS(BitMapTest1::A, GameLevel)

AWL_TEST(BitMapEnclosed)
{
    using namespace BitMapTest1;

    AWL_UNUSED_CONTEXT;

    A::Test();

    AWL_ASSERT(A({ }).m_bm.none());
    AWL_ASSERT((A({ A::GameLevel::Baby, A::GameLevel::Starter, A::GameLevel::Professional, A::GameLevel::Expert }).m_bm.all()));

    using GameLevelBitMap2 = awl::bitmap<A::GameLevel, A::GameLevelTraits::Count>;
    AWL_ASSERT(GameLevelBitMap2{ }.none());
    AWL_ASSERT((GameLevelBitMap2{ A::GameLevel::Baby, A::GameLevel::Starter, A::GameLevel::Professional, A::GameLevel::Expert }.all()));

    using GameLevelBitMap3 = awl::bitmap<A::GameLevel>;
    AWL_ASSERT(GameLevelBitMap3{ }.none());
    AWL_ASSERT((GameLevelBitMap3{ A::GameLevel::Baby, A::GameLevel::Starter, A::GameLevel::Professional, A::GameLevel::Expert }.all()));
}
