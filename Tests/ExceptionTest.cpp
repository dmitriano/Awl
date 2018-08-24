#include "Awl/Io/IoException.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;
using namespace awl::io;

static void Print(const TestContext & context, const awl::Exception & e)
{
    context.out << e.GetClassName() << _T(" ") << e.GetMessage() << std::endl;
}

AWL_TEST(ExceptionMessage)
{
    Print(context, EndOfFileException(5, 3));
    Print(context, TestException(_T("Test message.")));
}

AWL_TEST(ExceptionTryCatch)
{
    try
    {
        throw TestException(_T("Test message."));
    }
    catch (const std::exception & e)
    {
        context.out << awl::FromACString<awl::Char>(e.what()) << std::endl;

        auto * p_awl_e = dynamic_cast<const awl::Exception *>(&e);
        
        if (p_awl_e)
        {
            Print(context, *p_awl_e);
        }
    }
}
