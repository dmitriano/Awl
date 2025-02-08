#include "QtExtras/Io/Rw/ReadWrite.h"

#include <Awl/Testing/UnitTest.h>

#include "Awl/Io/VectorStream.h"

namespace
{
    const char sample[] = "some string";

    template <class T>
    void Write(std::vector<uint8_t>& v, const T& val)
    {
        awl::io::VectorOutputStream out(v);

        awl::io::Write(out, val);
    }

    template <class T>
    void Read(const std::vector<uint8_t>& v, const T& expected)
    {
        awl::io::VectorInputStream in(v);

        T val;

        awl::io::Read(in, val);

        AWT_ASSERT(val == expected);
    }
}

AWT_TEST(RwQtString)
{
    AWT_UNUSED_CONTEXT;

    {
        std::vector<uint8_t> v;

        Write(v, std::string(sample));
        Read(v, QString(sample));
    }

    {
        std::vector<uint8_t> v;

        Write(v, QString(sample));
        Read(v, std::string(sample));
    }
}
