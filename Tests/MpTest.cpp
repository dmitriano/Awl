/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Mp/TypeCollector.h"
#include "Awl/Mp/VariantFromStructs.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"

#include "VtsData.h"

#include <vector>
#include <map>
#include <list>
#include <string>

using namespace awl::testing::helpers;

namespace
{
    struct D
    {
        v1::A a;
        v1::C c;

        AWL_REFLECT(a, c)
    };
}

namespace awl::mp
{
    static_assert(make_type_name<std::string>() == std::string("sequence<int8_t>"));
    //wstring is int32_t in GCC and uint16_t in MSVC
    //static_assert(make_type_name<std::wstring>() == std::string("sequence<int16_t>"));
    static_assert(make_type_name<std::vector<int32_t>>() == std::string("sequence<int32_t>"));
    static_assert(make_type_name<std::vector<std::list<uint64_t>>>() == std::string("sequence<sequence<int64_t>>"));

    static_assert(make_type_name<std::map<int32_t, int64_t>>() == std::string("sequence<pair<int32_t, int64_t>>"));

    static_assert(make_type_name<std::optional<std::string>>() == std::string("optional<sequence<int8_t>>"));

    static_assert(std::is_same_v<type_collector<std::vector<int>>::Tuple,
        std::tuple<
            std::vector<int>>>);

    // std::string is also a range.
    static_assert(std::is_same_v<type_collector<std::pair<const std::string, int>>::Tuple,
        std::tuple<
            std::string, int>>);

    static_assert(std::is_same_v<type_collector<std::map<std::string, int>>::Tuple, 
        std::tuple<
            std::map<std::string, int>,
            std::string,
            int>>);

    static_assert(std::is_same_v<type_collector<v1::A>::Tuple,
        std::tuple<
            v1::A,
            int,
            bool,
            std::string,
            double>>);

    static_assert(std::is_same_v<type_collector<D>::Tuple,
        std::tuple<
            D,
            v1::A,
            int,
            bool,
            std::string,
            double,
            v1::C,
            int,
            v1::A,
            int,
            bool,
            std::string,
            double
        >>);
}

AWL_TEST(TypeCollector)
{
    context.logger.debug(awl::format() << awl::FromAString(typeid(awl::mp::type_collector<std::pair<const std::string, int>>::Tuple).name()) << awl::format::endl << awl::format::endl);

    context.logger.debug(awl::format() << awl::FromAString(typeid(awl::mp::type_collector<std::map<const std::string, int>>::Tuple).name()) << awl::format::endl << awl::format::endl);

    context.logger.debug(awl::format() << awl::FromAString(typeid(awl::mp::type_collector<v1::A>::Tuple).name()) << awl::format::endl << awl::format::endl);

    context.logger.debug(awl::format() << awl::FromAString(typeid(awl::mp::type_collector<v1::B>::Tuple).name()) << awl::format::endl << awl::format::endl);

    context.logger.debug(awl::format() << awl::FromAString(typeid(awl::mp::type_collector<D>::Tuple).name()) << awl::format::endl << awl::format::endl);

    // class std::tuple<
    // struct `anonymous namespace'::D,
    // struct awl::testing::helpers::v1::A,
    // int,
    // bool,
    // class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >,
    // char,
    // double,
    // struct awl::testing::helpers::v1::C,
    // int,
    // struct awl::testing::helpers::v1::A,
    // int,
    // bool,
    // class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >,
    // char,
    // double>
}

AWL_TEST(VariantFromStructs)
{
    context.logger.debug(awl::format() << awl::FromAString(typeid(awl::mp::variant_from_struct<v1::A>).name()));
}
