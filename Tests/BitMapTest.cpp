#include "Awl/BitMap.h"

#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

AWL_SEQUENTIAL_ENUM(Vehicle, Car, Train, AirPlain)

static_assert(awl::EnumTraits<Vehicle>::count() == 3);

AWL_TEST(BitMap)
{
    AWL_UNUSED_CONTEXT;

    const awl::bitmap<Vehicle> car{ Vehicle::Car };

    Assert::IsTrue(car.count() == 1);
    Assert::IsTrue(car.size() == 3);

    Assert::IsTrue(car);
    
    Assert::IsTrue(car[Vehicle::Car]);
    Assert::IsTrue(car & Vehicle::Car);
    Assert::IsTrue(car | Vehicle::Car);
    Assert::IsFalse(car ^ Vehicle::Car);

    Assert::IsFalse(car[Vehicle::Train]);
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
