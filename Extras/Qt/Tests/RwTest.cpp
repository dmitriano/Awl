/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#include "QtExtras/Io/Rw/ReadWrite.h"

#include <Awl/Testing/UnitTest.h>
#include "Awl/Io/VectorStream.h"

#include <cstddef>

namespace
{
    const char sample[] = "some string";

    template <class T>
    void write(std::vector<std::byte>& v, const T& val)
    {
        awl::io::VectorOutputStream out(v);

        awl::io::write(out, val);
    }

    template <class T>
    void read(const std::vector<std::byte>& v, const T& expected)
    {
        awl::io::VectorInputStream in(v);

        T val;

        awl::io::read(in, val);

        AWL_ASSERT(val == expected);
    }
}

AWL_TEST(RwQtString)
{
    AWL_UNUSED_CONTEXT;

    {
        std::vector<std::byte> v;

        write(v, std::string(sample));
        read(v, QString(sample));
    }

    {
        std::vector<std::byte> v;

        write(v, QString(sample));
        read(v, std::string(sample));
    }
}
