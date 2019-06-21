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

AWT_TEST(TransformTuple)
{
    AWT_UNUSED_CONTEXT;

    const auto t = std::make_tuple(1, 2.0, std::string("abc"));

    const auto wt = awl::transform(t, [](const auto & field) { return FieldWriter(field); });

    const auto writers = awl::to_array(wt, [](const auto & field_writer) { return static_cast<const Writer *>(&field_writer); });

    std::ostringstream out;

    for (const auto * p_writer : writers)
    {
        p_writer->Write(out);
    }

    std::string result = out.str();
    
    Assert::IsTrue(result == "1 2 abc ");

    std::remove_const_t<decltype(t)> t1{};

    const auto rt = awl::transform(t1, [](auto & field) { return FieldReader(field); });

    const auto readers = awl::to_array(rt, [](auto & field_reader) { return static_cast<const Reader *>(&field_reader); });

    std::istringstream in(result);
    
    for (auto * p_reader : readers)
    {
        p_reader->Read(in);
    }

    Assert::IsTrue(t1 == t);
}

AWT_TEST(MapTuple)
{
    AWT_UNUSED_CONTEXT;

    typedef std::tuple<bool, char, int, float, double, std::string> T1;
    typedef std::tuple<char, int, int, double, std::string, int, double> T2;

    auto a = awl::map_types<T1, T2>();

    Assert::IsTrue(a == std::array<size_t, std::tuple_size<T2>::value>{1, 2, 2, 4, 5, 2, 4});
}
