#include <string>
#include <sstream>

#include "Awl/TupleHelpers.h"

#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

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

AWT_TEST(TupleTransform)
{
    AWT_UNUSED_CONTEXT;

    const auto t = std::make_tuple(1, 2.0, std::string("abc"));

    const auto wt = awl::transform_tuple(t, [](const auto & field) { return FieldWriter(field); });

    const auto writers = awl::tuple_to_array(wt, [](const auto & field_writer) { return static_cast<const Writer *>(&field_writer); });

    std::ostringstream out;

    for (const auto * p_writer : writers)
    {
        p_writer->Write(out);
    }

    std::string result = out.str();
    
    Assert::IsTrue(result == "1 2 abc ");

    std::remove_const_t<decltype(t)> t1{};

    const auto rt = awl::transform_tuple(t1, [](auto & field) { return FieldReader(field); });

    const auto readers = awl::tuple_to_array(rt, [](auto & field_reader) { return static_cast<const Reader *>(&field_reader); });

    std::istringstream in(result);
    
    for (auto * p_reader : readers)
    {
        p_reader->Read(in);
    }

    Assert::IsTrue(t1 == t);
}

AWT_TEST(TupleMapT2T)
{
    AWT_UNUSED_CONTEXT;

    typedef std::tuple<bool, char, int, float, double, std::string> T1;
    const auto result = std::array<size_t, 7>{1, 2, 2, 4, 5, 2, 4};

    {
        typedef std::tuple<char, int, int, double, std::string, int, double> T2;

        static_assert(awl::find_tuple_type_v<bool, T1> == 0);
        static_assert(awl::find_tuple_type_v<char, T1> == 1);
        static_assert(awl::find_tuple_type_v<std::string, T1> == 5);

        auto a = awl::map_types_t2t<T2, T1>();

        Assert::IsTrue(a == result);
    }

    {
        typedef std::tuple<char&, int&, int, double&, std::string&, int, double&> T2;

        static_assert(awl::find_tuple_type_v<bool&, T1> == 0);
        static_assert(awl::find_tuple_type_v<char&, T1> == 1);
        static_assert(awl::find_tuple_type_v<std::string&, T1> == 5);

        auto a = awl::map_types_t2t<T2, T1>();

        Assert::IsTrue(a == result);
    }
}

AWT_TEST(TupleMapT2V)
{
    AWT_UNUSED_CONTEXT;

    typedef std::variant<bool, char, int, float, double, std::string> T1;
    const auto result = std::array<size_t, 7>{1, 2, 2, 4, 5, 2, 4};

    {
        typedef std::tuple<char, int, int, double, std::string, int, double> T2;

        static_assert(awl::find_variant_type_v<bool, T1> == 0);
        static_assert(awl::find_variant_type_v<char, T1> == 1);
        static_assert(awl::find_variant_type_v<std::string, T1> == 5);

        auto a = awl::map_types_t2v<T2, T1>();

        Assert::IsTrue(a == result);
    }

    {
        typedef std::tuple<char&, int&, int, double&, std::string&, int, double&> T2;

        static_assert(awl::find_variant_type_v<bool&, T1> == 0);
        static_assert(awl::find_variant_type_v<char&, T1> == 1);
        static_assert(awl::find_variant_type_v<std::string&, T1> == 5);

        auto a = awl::map_types_t2v<T2, T1>();

        Assert::IsTrue(a == result);
    }
}
