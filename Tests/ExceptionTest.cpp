#include "Awl/Io/IoException.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;
using namespace awl::io;

static void Print(const TestContext & context, const awl::Exception & e)
{
    context.out << e.GetClassName() << _T(" ") << e.GetMessage() << std::endl;
}

AWL_TEST(Exception)
{
    Print(context, EndOfFileException(5, 3));
    Print(context, TestException(_T("Test message.")));
}
