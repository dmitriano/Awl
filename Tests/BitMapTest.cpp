#include "Awl/BitMap.h"

#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

namespace BitMapTest
{
    AWL_SEQUENTIAL_ENUM(Vehicle, Car, Train, AirPlain)
    using ConstBitMap = const awl::bitmap<Vehicle>;
}

AWL_ENUM_TRAITS(BitMapTest, Vehicle)

namespace BitMapTest
{
    static_assert(awl::EnumTraits<Vehicle>::count() == 3);
    static_assert(ConstBitMap{ Vehicle::Car }[Vehicle::Car]);
    static_assert(!ConstBitMap{ Vehicle::Car }[Vehicle::Train]);
    static_assert(ConstBitMap{ Vehicle::Car, Vehicle::Train }[Vehicle::Train]);
}

AWL_TEST(BitMap)
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
