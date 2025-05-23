/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <sstream>
#include <atomic>

#include "Awl/TupleHelpers.h"

#include "Awl/Testing/UnitTest.h"

#include "Helpers/NonCopyable.h"

using namespace awl::testing;

namespace
{
    struct Writer
    {
        virtual void Write(std::ostream & out) const = 0;
    };

    struct Reader
    {
        virtual void Read(std::istream & in) const = 0;
    };

    template <class Field>
    class FieldWriter : public Writer
    {
    public:

        FieldWriter(const Field & field) : m_field(field)
        {
        }

        void Write(std::ostream & out) const override
        {
            out << m_field << " ";
        }

    private:

        const Field & m_field;
    };

    template <class Field>
    class FieldReader : public Reader
    {
    public:

        FieldReader(Field & field) : m_field(field)
        {
        }

        void Read(std::istream & in) const override
        {
            in >> m_field;
        }

    private:

        Field & m_field;
    };
}

AWL_TEST(TupleTransform)
{
    AWL_UNUSED_CONTEXT;

    const auto t = std::make_tuple(1, 2.0, std::string("abc"));

    const auto wt = awl::transform_tuple(t, [](const auto & field) { return FieldWriter(field); });

    const auto writers = awl::tuple_cast<const Writer>(wt);

    std::ostringstream out;

    for (const auto * p_writer : writers)
    {
        p_writer->Write(out);
    }

    std::string result = out.str();
    
    AWL_ASSERT(result == "1 2 abc ");

    std::remove_const_t<decltype(t)> t1{};

    const auto rt = awl::transform_tuple(t1, [](auto & field) { return FieldReader(field); });

    const auto readers = awl::tuple_cast<const Reader>(rt);

    std::istringstream in(result);
    
    for (auto * p_reader : readers)
    {
        p_reader->Read(in);
    }

    AWL_ASSERT(t1 == t);
}

AWL_TEST(TupleMapT2T)
{
    AWL_UNUSED_CONTEXT;

    using T1 = std::tuple<bool, char, int, float, double, std::string>;
    const auto result = std::array<size_t, 7>{1, 2, 2, 4, 5, 2, 4};

    {
        using T2 = std::tuple<char, int, int, double, std::string, int, double>;

        static_assert(awl::find_tuple_type_v<bool, T1> == 0);
        static_assert(awl::find_tuple_type_v<char, T1> == 1);
        static_assert(awl::find_tuple_type_v<std::string, T1> == 5);

        auto a = awl::map_types_t2t<T2, T1>();

        AWL_ASSERT(a == result);
    }

    {
        using T2 = std::tuple<char&, int&, int, double&, std::string&, int, double&>;

        static_assert(awl::find_tuple_type_v<bool&, T1> == 0);
        static_assert(awl::find_tuple_type_v<char&, T1> == 1);
        static_assert(awl::find_tuple_type_v<std::string&, T1> == 5);

        auto a = awl::map_types_t2t<T2, T1>();

        AWL_ASSERT(a == result);
    }
}

AWL_TEST(TupleMapT2V)
{
    AWL_UNUSED_CONTEXT;

    using T1 = std::variant<bool, char, int, float, double, std::string>;
    const auto result = std::array<size_t, 7>{1, 2, 2, 4, 5, 2, 4};

    {
        using T2 = std::tuple<char, int, int, double, std::string, int, double>;

        static_assert(awl::find_variant_type_v<bool, T1> == 0);
        static_assert(awl::find_variant_type_v<char, T1> == 1);
        static_assert(awl::find_variant_type_v<std::string, T1> == 5);

        auto a = awl::map_types_t2v<T2, T1>();

        AWL_ASSERT(a == result);
    }

    {
        using T2 = std::tuple<char&, int&, int, double&, std::string&, int, double&>;

        static_assert(awl::find_variant_type_v<bool&, T1> == 0);
        static_assert(awl::find_variant_type_v<char&, T1> == 1);
        static_assert(awl::find_variant_type_v<std::string&, T1> == 5);

        auto a = awl::map_types_t2v<T2, T1>();

        AWL_ASSERT(a == result);
    }
}

AWL_TEST(TupleMakeUniversal)
{
    AWL_UNUSED_CONTEXT;

    {
        std::string a("a");

        const std::string b("b");

        auto t = std::make_tuple(a, b, 1);

        auto universal_t = awl::make_universal_tuple(a, b, 1);

        using Tuple = decltype(t);

        using UniversalTuple = decltype(universal_t);

        static_assert(std::is_same_v<Tuple, std::tuple<std::string, std::string, int>>);

        static_assert(std::is_same_v<UniversalTuple, std::tuple<std::string&, const std::string&, int>>);

        // Wow... we can convert and compare them.

        //GCC and CLang do not compile this.
#if defined(_MSC_VER)
        UniversalTuple universal_t1 = t;

        AWL_ASSERT(universal_t1 == t);
#endif

        using ConstRefTuple = std::tuple<const std::string&, const std::string&, const int&>;

        ConstRefTuple const_t = t;

        AWL_ASSERT(const_t == t);
    }

    {
        auto universal_t = awl::make_universal_tuple(std::string("a"), 1);
        using UniversalTuple = decltype(universal_t);
        static_assert(std::is_same_v<UniversalTuple, std::tuple<std::string, int>>);
    }
}

AWL_TEST(TupleMakeSimilar)
{
    AWL_UNUSED_CONTEXT;

    using Tuple = std::tuple<std::string, int>;

    {
        int i = 10;
        auto t = awl::make_similar_tuple<Tuple>("a", i);
        static_assert(std::is_same_v<decltype(t), std::tuple<std::string, int&>>);
        AWL_ASSERT(t == std::make_tuple(std::string("a"), i));
    }

    std::string s = "b";
    
    {
        const int j = 5;
        auto t = awl::make_similar_tuple<Tuple>(std::string{}, j);
        static_assert(std::is_same_v<decltype(t), std::tuple<std::string, const int&>>);
        AWL_ASSERT(t == std::make_tuple(std::string{}, j));
    }

    {
        auto t = awl::make_similar_tuple<Tuple>(s, int{});
        static_assert(std::is_same_v<decltype(t), std::tuple<std::string&, int>>);
        AWL_ASSERT(t == std::make_tuple(s, int{}));
    }

    {
        auto t = awl::make_similar_tuple<Tuple>(s, int8_t{});
        static_assert(std::is_same_v<decltype(t), std::tuple<std::string&, int>>);
        AWL_ASSERT(t == std::make_tuple(s, int{}));
    }
    
    {
        auto t = awl::make_similar_tuple<Tuple>("c", int8_t{});
        static_assert(std::is_same_v<decltype(t), std::tuple<std::string, int>>);
        AWL_ASSERT(t == std::make_tuple(std::string("c"), int{}));
    }

    {
        auto t = awl::make_similar_tuple<Tuple>(std::as_const(s), int{});
        static_assert(std::is_same_v<decltype(t), std::tuple<const std::string&, int>>);
        AWL_ASSERT(t == std::make_tuple(s, int{}));
    }
}

AWL_TEST(TupleNonCopyable)
{
    AWL_UNUSED_CONTEXT;

    awl::tuple_cat_t<std::tuple<std::atomic<int>>, std::tuple<bool>> t1;
    static_cast<void>(t1);

    awl::tuple_make_t<std::atomic<int>, bool> t2;
    static_cast<void>(t2);

    // With MSVC I'll get 'no overloaded function could convert all the argument types'.
    // This is why the serilaization uses tuples of refereces, but not values.
    // using A = std::atomic<int>;

    using A = awl::testing::helpers::NonCopyable;

    A a(5);

    std::tuple<A> t(std::move(a));
}

AWL_TEST(TupleDiff)
{
    AWL_UNUSED_CONTEXT;

    using Tuple = std::tuple<int, std::string, bool>;

    Tuple a(5, "a", true);
    Tuple b(5, "b", true);

    const std::array expected_aa{ false, false, false };
    const std::array expected_ab{ false, true, false };

    AWL_ASSERT(awl::tuple_diff(a, a) == expected_aa);
    AWL_ASSERT(awl::tuple_diff(a, b) == expected_ab);
}
