/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/IoException.h"
#include "Awl/Testing/UnitTest.h"

#include <locale.h>

using namespace awl::testing;
using namespace awl::io;

static void Print(const TestContext & context, const awl::Exception & e)
{
    context.out << e.GetClassName() << _T(" ") << e.What() << std::endl;
}

static void EncodeDecode(const TestContext &, const std::wstring sample)
{
    const std::string encoded = awl::EncodeString(sample.c_str());

    const std::wstring decoded = awl::DecodeString(encoded.c_str());

    AWT_ASSERT(decoded == sample);
}

AWT_TEST(DecodeString)
{
    setlocale(LC_ALL, "ru_RU.utf8");

    {
        const char * encoded = reinterpret_cast<const char*>(u8"z\u00df\u6c34\U0001f34c");

        context.out << _T("Decoded string: ") << awl::FromACString(encoded) << std::endl;
    }

    EncodeDecode(context, L"");
}

AWT_TEST(ExceptionMessage)
{
    setlocale(LC_ALL, "en_US.utf8");

    Print(context, EndOfFileException(5, 3));
    Print(context, TestException(_T("Test message.")));
}

AWT_TEST(ExceptionTryCatch)
{
    setlocale(LC_ALL, "en_US.utf8");

    try
    {
        throw TestException(_T("Test message."));
    }
    catch (const std::exception & e)
    {
        AWT_ASSERT(strcmp(e.what(), typeid(TestException).name()) == 0);

        context.out << awl::FromACString(e.what()) << std::endl;

        auto * p_awl_e = dynamic_cast<const awl::Exception *>(&e);
        
        if (p_awl_e)
        {
            Print(context, *p_awl_e);
        }
    }
}

AWT_TEST(ExceptionClassName)
{
    {
        EndOfFileException e(5, 3);
        Print(context, e);
        AWT_ASSERT(strcmp(e.what(), typeid(EndOfFileException).name()) == 0);
    }

    {
        TestException e(_T("abc"));
        Print(context, e);
        AWT_ASSERT(strcmp(e.what(), typeid(TestException).name()) == 0);
    }
}
