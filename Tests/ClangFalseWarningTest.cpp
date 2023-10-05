/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Stringizable.h"

#include "Awl/Testing/UnitTest.h"

namespace
{
    template <class T>
    void Read(T& val)
    {
        static_cast<void>(val);
    }

    class TestReader
    {
    public:

        template<class Struct>
        void ReadV(Struct & val) const
        {
            if constexpr (awl::is_tuplizable_v<Struct>)
            {
                awl::for_each(awl::object_as_tuple(val), [this, &val](auto& field)
                {
                    ReadV(field);
                });
            }
            else
            {
                Read(val);
            }
        }
    };

    struct A
    {
        int x;

        AWL_STRINGIZABLE(x)
    };
}

AWT_TEST(CLangFalseWarning)
{
    AWT_UNUSED_CONTEXT;

    TestReader reader;

    A a;
    reader.ReadV(a);
}
