#include "BoostExtras/Json/JsonUtil.h"
#include "Tests/VtsData.h"

#include "Awl/Decimal128.h"
#include "Awl/EnumTraits.h"
#include "Awl/Testing/UnitTest.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace awl::testing::boost_json_tests
{
    AWL_SEQUENTIAL_ENUM(TestEnum, first, second, third)
}

AWL_ENUM_TRAITS(awl::testing::boost_json_tests, TestEnum)

using namespace awl::testing::helpers::v1;
namespace json = awl;

namespace
{
    boost::json::object makeAJson()
    {
        boost::json::object jo;
        jo["a"] = a_expected.a;
        jo["b"] = a_expected.b;
        jo["c"] = a_expected.c;
        jo["d"] = a_expected.d;
        return jo;
    }

    boost::json::object makeCJson()
    {
        boost::json::object jo;
        jo["x"] = c_expected.x;
        jo["a"] = makeAJson();
        return jo;
    }

    boost::json::object makeBJson()
    {
        boost::json::object jo;
        jo["a"] = makeAJson();
        jo["b"] = makeAJson();
        jo["x"] = b_expected.x;
        jo["y"] = b_expected.y;
        jo["v"] = boost::json::array{ makeAJson(), makeAJson(), makeAJson() };
        jo["v1"] = boost::json::array{ makeCJson() };
        return jo;
    }
}

AWL_TEST(BoostJsonArithmeticSerializer)
{
    AWL_UNUSED_CONTEXT;
    boost::json::value jv;

    json::toJson(42, jv);
    AWL_ASSERT(jv.as_int64() == 42);

    int int_val{};
    json::fromJson(jv, int_val);
    AWL_ASSERT(int_val == 42);

    bool bool_val{};
    json::fromJson(boost::json::value(true), bool_val);
    AWL_ASSERT(bool_val);

    double double_val{};
    json::fromJson(boost::json::value("12.5"), double_val);
    AWL_ASSERT(double_val == 12.5);
}

AWL_TEST(BoostJsonAtomicSerializer)
{
    AWL_UNUSED_CONTEXT;
    std::atomic<int> value{ 7 };

    boost::json::value jv = json::toJson(value);
    AWL_ASSERT(jv.as_int64() == 7);

    json::fromJson(boost::json::value(11), value);
    AWL_ASSERT(value.load() == 11);
}

AWL_TEST(BoostJsonStringSerializer)
{
    AWL_UNUSED_CONTEXT;
    std::string text;
    json::fromJson(boost::json::value("abc"), text);
    AWL_ASSERT(text == "abc");

    boost::json::value jv = json::toJson(std::string("xyz"));
    AWL_ASSERT(jv.as_string() == "xyz");
}

AWL_TEST(BoostJsonOptionalSerializer)
{
    AWL_UNUSED_CONTEXT;
    std::optional<int> value;
    json::fromJson(boost::json::value(nullptr), value);
    AWL_ASSERT(!value);

    json::fromJson(boost::json::value(5), value);
    AWL_ASSERT(value == 5);

    boost::json::value jv = json::toJson(value);
    AWL_ASSERT(jv.as_int64() == 5);
}

AWL_TEST(BoostJsonSequenceSerializer)
{
    AWL_UNUSED_CONTEXT;
    std::vector<int> values;
    json::fromJson(boost::json::array{ 1, 2, 3 }, values);
    AWL_ASSERT(values == std::vector<int>({ 1, 2, 3 }));

    std::set<int> set_values;
    json::fromJson(boost::json::array{ 3, 1, 2 }, set_values);
    AWL_ASSERT(set_values == std::set<int>({ 1, 2, 3 }));

    boost::json::value jv = json::toJson(values);
    AWL_ASSERT(jv.as_array().size() == 3);
}

AWL_TEST(BoostJsonMapSerializer)
{
    AWL_UNUSED_CONTEXT;
    std::map<std::string, int> expected_map{ { "a", 1 }, { "b", 2 } };
    boost::json::value jv = json::toJson(expected_map);

    std::map<std::string, int> actual_map;
    json::fromJson(jv, actual_map);
    AWL_ASSERT(actual_map == expected_map);

    std::unordered_map<std::string, int> unordered_map;
    json::fromJson(jv, unordered_map);
    AWL_ASSERT(unordered_map.at("a") == 1);
    AWL_ASSERT(unordered_map.at("b") == 2);
}

AWL_TEST(BoostJsonReflectableSerializer)
{
    AWL_UNUSED_CONTEXT;
    A a;
    json::fromJson(makeAJson(), a);
    AWL_ASSERT(a == a_expected);

    B b;
    json::fromJson(makeBJson(), b);
    AWL_ASSERT(b == b_expected);
}

AWL_TEST(BoostJsonReflectableExceptionPath)
{
    AWL_UNUSED_CONTEXT;
    boost::json::object b_jo = makeBJson();
    boost::json::object a_jo = makeAJson();
    a_jo["b"] = "text";
    b_jo["a"] = a_jo;

    B b;

    try
    {
        json::fromJson(b_jo, b);
        AWL_FAILM(_T("Exception of type JsonException was not thrown."));
    }
    catch (const json::JsonException& e)
    {
        context.logger->debug(e.message());
    }
}

AWL_TEST(BoostJsonEnumSerializer)
{
    AWL_UNUSED_CONTEXT;
    using awl::testing::boost_json_tests::TestEnum;

    boost::json::value jv = json::toJson(TestEnum::second);
    AWL_ASSERT(jv.as_string() == "second");

    TestEnum value{};
    json::fromJson(jv, value);
    AWL_ASSERT(value == TestEnum::second);
}

AWL_TEST(BoostJsonTupleSerializer)
{
    AWL_UNUSED_CONTEXT;
    std::tuple<int, std::string, bool> value{ 1, "abc", true };
    boost::json::value jv = json::toJson(value);

    std::tuple<int, std::string, bool> actual;
    json::fromJson(jv, actual);
    AWL_ASSERT(actual == value);
}

AWL_TEST(BoostJsonReferenceWrapperSerializer)
{
    AWL_UNUSED_CONTEXT;
    int value = 3;
    std::reference_wrapper<int> ref(value);

    json::fromJson(boost::json::value(9), ref);
    AWL_ASSERT(value == 9);

    boost::json::value jv = json::toJson(ref);
    AWL_ASSERT(jv.as_int64() == 9);
}

AWL_TEST(BoostJsonJsonSerializer)
{
    AWL_UNUSED_CONTEXT;
    boost::json::value source = boost::json::object{ { "a", 1 } };
    boost::json::value copied;
    json::fromJson(source, copied);
    AWL_ASSERT(copied == source);

    boost::json::object object;
    json::fromJson(source, object);
    AWL_ASSERT(object.at("a").as_int64() == 1);

    boost::json::array array;
    json::fromJson(boost::json::array{ 1, 2 }, array);
    AWL_ASSERT(array.size() == 2);
}

AWL_TEST(BoostJsonRangeSerializer)
{
    AWL_UNUSED_CONTEXT;
    std::vector<int> values{ 1, 2, 3 };
    boost::json::array array = json::rangeToJson(values);
    AWL_ASSERT(array.size() == values.size());
    AWL_ASSERT(array[0].as_int64() == 1);
}

AWL_TEST(BoostJsonTimeSerializer)
{
    AWL_UNUSED_CONTEXT;
    using namespace std::chrono;

    system_clock::time_point tp(milliseconds(12345));
    boost::json::value tp_json = json::toJson(tp);

    system_clock::time_point actual_tp;
    json::fromJson(tp_json, actual_tp);
    AWL_ASSERT(actual_tp == tp);

    milliseconds duration;
    json::fromJson(boost::json::value("2d.3h.4m.5s.006"), duration);
    AWL_ASSERT(duration == days(2) + hours(3) + minutes(4) + seconds(5) + milliseconds(6));

    boost::json::value duration_json = json::toJson(duration);
    AWL_ASSERT(duration_json.as_string() == "2d.3h.4m.5s.006");
}

#ifdef AWL_DECIMAL_128

AWL_TEST(BoostJsonDecimalSerializer)
{
    AWL_UNUSED_CONTEXT;
    using Decimal = awl::decimal128<4>;

    Decimal value;
    json::fromJson(boost::json::value("123.4567"), value);
    AWL_ASSERT(value == Decimal("123.4567"));

    boost::json::value jv = json::toJson(value);
    AWL_ASSERT(jv.as_string() == "123.4567");
}

#endif

AWL_TEST(BoostJsonUtil)
{
    AWL_UNUSED_CONTEXT;
    B value;
    json::structFromString(json::jsonToString(makeBJson()), value);
    AWL_ASSERT(value == b_expected);

    const std::string text = json::structToString(value);
    AWL_ASSERT(!text.empty());
}
