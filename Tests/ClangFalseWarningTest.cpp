/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/UnitTest.h"

namespace
{
    template<typename Test, template<typename...> class Ref>
    struct is_specialization : std::false_type {};

    template<template<typename...> class Ref, typename... Args>
    struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

    template<typename Test, template<typename...> class Ref>
    constexpr bool is_specialization_v = is_specialization<Test, Ref>::value;

    template <typename... Args, typename Func, std::size_t... index>
    constexpr void for_each(const std::tuple<Args...>& t, const Func& f, std::index_sequence<index...>)
    {
        (f(std::get<index>(t)), ...);
    }

    template <typename... Args, typename Func>
    constexpr void for_each(const std::tuple<Args...>& t, const Func& f)
    {
        for_each(t, f, std::index_sequence_for<Args...>{});
    }

    class TestReader
    {
    public:

        template<class Struct>
        void ReadV(Struct& val) const
        {
            if constexpr (is_specialization_v<Struct, std::tuple>)
            {
                for_each(val, [this](auto& field)
                {
                    //Warining with Android CLang 17.0.2 from NDK 26.0.10792818:
                    //lambda capture 'this' is not used [-Wunused-lambda-capture]
                    ReadV(field);
                });
            }
            else
            {
                static_cast<void>(val);
            }
        }
    };
}

AWL_TEST(CLangFalseWarning)
{
    AWL_UNUSED_CONTEXT;

    TestReader reader;

    std::tuple<int> a(1);
    reader.ReadV(a);
}
