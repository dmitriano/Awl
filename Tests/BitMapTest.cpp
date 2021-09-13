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
}

AWT_TEST(BitMapWithEnumTraits)
{
    AWT_UNUSED_CONTEXT;
    
    using namespace BitMapTest;

    constexpr awl::bitmap<Vehicle> car{ Vehicle::Car };

    AWT_ASSERT(car.count() == 1);
    static_assert(car.size() == 3);

    AWT_ASSERT(car);
    
    static_assert(car[Vehicle::Car]);
    AWT_ASSERT(car & Vehicle::Car);
    AWT_ASSERT(car | Vehicle::Car);
    AWT_ASSERT_FALSE(car ^ Vehicle::Car);

    static_assert(!car[Vehicle::Train]);
    AWT_ASSERT_FALSE(car & Vehicle::Train);
    AWT_ASSERT_FALSE(car & Vehicle::Train);
    AWT_ASSERT(car | Vehicle::Train);
    AWT_ASSERT((car ^ Vehicle::Train) & Vehicle::Car);

    const awl::bitmap<Vehicle> other = ~car;

    AWT_ASSERT_FALSE(other[Vehicle::Car]);
    AWT_ASSERT_FALSE(other & Vehicle::Car);
    AWT_ASSERT(other | Vehicle::Car);
    AWT_ASSERT(other ^ Vehicle::Car);

    AWT_ASSERT(other[Vehicle::Train] && other[Vehicle::AirPlain]);
    AWT_ASSERT((other & Vehicle::Train) && (other & Vehicle::AirPlain));
    AWT_ASSERT_FALSE(other & Vehicle::Car);
    AWT_ASSERT(other | Vehicle::Car);
    AWT_ASSERT_FALSE((other ^ Vehicle::Train) & Vehicle::Car);

    const awl::bitmap<Vehicle> all = car | other;
    AWT_ASSERT(all.all());
    AWT_ASSERT(all == (car ^ other));
    AWT_ASSERT_FALSE(car & other);

    const awl::bitmap<Vehicle> none = ~all;
    AWT_ASSERT(none.none());
    AWT_ASSERT_FALSE(none);

    AWT_ASSERT(all == ~none);

    awl::bitmap<Vehicle> var;
    AWT_ASSERT_FALSE(var);
    var[Vehicle::Car] = true;
    AWT_ASSERT(var[Vehicle::Car]);
    AWT_ASSERT_FALSE(var[Vehicle::Train]);
    AWT_ASSERT_FALSE(var[Vehicle::AirPlain]);
    AWT_ASSERT(var == car);
    AWT_ASSERT(var != other);

    //There is no operator == (bool), but this compiles.
    AWT_ASSERT(var == true);
    AWT_ASSERT(none == false);
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
            AWT_ASSERT(GameLevel1BitMap{}.none());
            AWT_ASSERT((GameLevel1BitMap{ GameLevel1::Baby, GameLevel1::Starter, GameLevel1::Professional, GameLevel1::Expert }.all()));
        }

        //error: Cannot specialize template in current scope.
        //AWL_ENUM_TRAITS(A, GameLevel)
    };
}

AWL_ENUM_TRAITS(BitMapTest1::A, GameLevel)

AWT_TEST(BitMapEnclosed)
{
    using namespace BitMapTest1;

    AWT_UNUSED_CONTEXT;

    A::Test();

    AWT_ASSERT(A({ }).m_bm.none());
    AWT_ASSERT((A({ A::GameLevel::Baby, A::GameLevel::Starter, A::GameLevel::Professional, A::GameLevel::Expert }).m_bm.all()));

    using GameLevelBitMap2 = awl::bitmap<A::GameLevel, A::GameLevelTraits::m_count>;
    AWT_ASSERT(GameLevelBitMap2{ }.none());
    AWT_ASSERT((GameLevelBitMap2{ A::GameLevel::Baby, A::GameLevel::Starter, A::GameLevel::Professional, A::GameLevel::Expert }.all()));

    using GameLevelBitMap3 = awl::bitmap<A::GameLevel>;
    AWT_ASSERT(GameLevelBitMap3{ }.none());
    AWT_ASSERT((GameLevelBitMap3{ A::GameLevel::Baby, A::GameLevel::Starter, A::GameLevel::Professional, A::GameLevel::Expert }.all()));
}
