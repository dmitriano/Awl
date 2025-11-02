/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/IoException.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"

#include <locale.h>

using namespace awl::testing;
using namespace awl::io;

static void Print(const TestContext & context, const awl::Exception & e)
{
    context.logger.debug(awl::format() << e.GetClassName() << _T(" ") << e.What());
}

static void EncodeDecode(const TestContext &, const std::wstring sample)
{
    const std::string encoded = awl::EncodeString(sample.c_str());

    const std::wstring decoded = awl::DecodeString(encoded.c_str());

    AWL_ASSERT(decoded == sample);
}

AWL_TEST(DecodeString)
{
    setlocale(LC_ALL, "ru_RU.utf8");

    {
        const char * encoded = reinterpret_cast<const char*>(u8"z\u00df\u6c34\U0001f34c");

        context.logger.debug(awl::format() << _T("Decoded string: ") << awl::FromACString(encoded));
    }

    EncodeDecode(context, L"");
}

AWL_TEST(ExceptionMessage)
{
    setlocale(LC_ALL, "en_US.utf8");

    Print(context, EndOfFileException(5, 3));
    Print(context, TestException(_T("Test message.")));
}

AWL_TEST(ExceptionTryCatch)
{
    setlocale(LC_ALL, "en_US.utf8");

    try
    {
        throw TestException(_T("Test message."));
    }
    catch (const std::exception & e)
    {
        AWL_ASSERT(strcmp(e.what(), typeid(TestException).name()) == 0);

        context.logger.debug(awl::format() << awl::FromACString(e.what()));

        auto * p_awl_e = dynamic_cast<const awl::Exception *>(&e);
        
        if (p_awl_e)
        {
            Print(context, *p_awl_e);
        }
    }
}

AWL_TEST(ExceptionClassName)
{
    {
        EndOfFileException e(5, 3);
        Print(context, e);
        AWL_ASSERT(strcmp(e.what(), typeid(EndOfFileException).name()) == 0);
    }

    {
        TestException e(_T("abc"));
        Print(context, e);
        AWL_ASSERT(strcmp(e.what(), typeid(TestException).name()) == 0);
    }
}
