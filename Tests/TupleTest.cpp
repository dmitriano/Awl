#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include "Awl/TupleHelpers.h"

#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

struct Writer
{
    virtual void Write(std::ostringstream & out) const = 0;
};

template <class Field>
class FieldWriter : public Writer
{
public:

    FieldWriter(const Field & field) : m_field(field)
    {
    }

    void Write(std::ostringstream & out) const override
    {
        out << m_field;
    }

private:
    
    const Field & m_field;
};

AWT_TEST(TransformTuple)
{
    AWT_UNUSED_CONTEXT;

    const auto t = std::make_tuple(1, 2.0, std::string("abc"));

    //const auto r = awl::transform(t, [&out](const auto & field) { return [&out, &field]() { out << field; }; });

    const auto r = awl::transform(t, [](const auto & field) { return FieldWriter(field); });

    const auto a = awl::to_array(r, [](const auto & field_writer) { return static_cast<const Writer *>(&field_writer); });

    std::ostringstream out;

    for (const auto * p_writer : a)
    {
        p_writer->Write(out);
    }

    std::string result = out.str();
    
    Assert::IsTrue(result == "12abc");
}
