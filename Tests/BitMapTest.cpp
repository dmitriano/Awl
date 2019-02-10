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

AWL_TEST(BitMapWithEnumTraits)
{
    using namespace BitMapTest;

    AWL_UNUSED_CONTEXT;

    constexpr awl::bitmap<Vehicle> car{ Vehicle::Car };

    Assert::IsTrue(car.count() == 1);
    static_assert(car.size() == 3);

    Assert::IsTrue(car);
    
    static_assert(car[Vehicle::Car]);
    Assert::IsTrue(car & Vehicle::Car);
    Assert::IsTrue(car | Vehicle::Car);
    Assert::IsFalse(car ^ Vehicle::Car);

    static_assert(!car[Vehicle::Train]);
    Assert::IsFalse(car & Vehicle::Train);
    Assert::IsFalse(car & Vehicle::Train);
    Assert::IsTrue(car | Vehicle::Train);
    Assert::IsTrue((car ^ Vehicle::Train) & Vehicle::Car);

    const awl::bitmap<Vehicle> other = ~car;

    Assert::IsFalse(other[Vehicle::Car]);
    Assert::IsFalse(other & Vehicle::Car);
    Assert::IsTrue(other | Vehicle::Car);
    Assert::IsTrue(other ^ Vehicle::Car);

    Assert::IsTrue(other[Vehicle::Train] && other[Vehicle::AirPlain]);
    Assert::IsTrue((other & Vehicle::Train) && (other & Vehicle::AirPlain));
    Assert::IsFalse(other & Vehicle::Car);
    Assert::IsTrue(other | Vehicle::Car);
    Assert::IsFalse((other ^ Vehicle::Train) & Vehicle::Car);

    const awl::bitmap<Vehicle> all = car | other;
    Assert::IsTrue(all.all());
    Assert::IsTrue(all == (car ^ other));
    Assert::IsFalse(car & other);

    const awl::bitmap<Vehicle> none = ~all;
    Assert::IsTrue(none.none());
    Assert::IsFalse(none);

    Assert::IsTrue(all == ~none);

    awl::bitmap<Vehicle> var;
    Assert::IsFalse(var);
    var[Vehicle::Car] = true;
    Assert::IsTrue(var[Vehicle::Car]);
    Assert::IsFalse(var[Vehicle::Train]);
    Assert::IsFalse(var[Vehicle::AirPlain]);
    Assert::IsTrue(var == car);
    Assert::IsTrue(var != other);

    //There is no operator == (bool), but this compiles.
    Assert::IsTrue(var == true);
    Assert::IsTrue(none == false);
}

namespace BitMapTest1
{
    class A
    {
    private:

        AWL_PRIVATE_BITMAP(GameLevel1, Baby, Starter, Professional, Expert)

    public:

        AWL_PUBLIC_BITMAP(GameLevel, Baby, Starter, Professional, Expert)

        A(GameLevelBitMap bm) : m_bm(bm)
        {
        }

        GameLevelBitMap m_bm;

        static void Test()
        {
            Assert::IsTrue(GameLevel1BitMap{}.none());
            Assert::IsTrue(GameLevel1BitMap{ GameLevel1::Baby, GameLevel1::Starter, GameLevel1::Professional, GameLevel1::Expert }.all());
        }
    };
}

AWL_TEST(BitMapEnclosed)
{
    using namespace BitMapTest1;

    AWL_UNUSED_CONTEXT;

    A::Test();

    Assert::IsTrue(A({ }).m_bm.none());
    Assert::IsTrue(A({ A::GameLevel::Baby, A::GameLevel::Starter, A::GameLevel::Professional, A::GameLevel::Expert }).m_bm.all());
}
