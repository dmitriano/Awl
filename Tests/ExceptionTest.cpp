#include "Awl/Io/IoException.h"
#include "Awl/Testing/UnitTest.h"

#include <locale.h>

using namespace awl::testing;
using namespace awl::io;

static void Print(const TestContext & context, const awl::Exception & e)
{
    context.out << e.GetClassName() << _T(" ") << e.GetMessage() << std::endl;
}

static void EncodeDecode(const TestContext & context, const std::wstring sample)
{
    const std::string encoded = awl::EncodeString(sample.c_str());

    const std::wstring decoded = awl::DecodeString(encoded.c_str());

    Assert::IsTrue(decoded == sample);
}

AWL_TEST(DecodeString)
{
    setlocale(LC_ALL, "ru_RU.utf8");

    {
        const char * encoded = u8"z\u00df\u6c34\U0001f34c";

        context.out << _T("Decoded string: ") << awl::FromACString(encoded) << std::endl;
    }

    EncodeDecode(context, L"");
    EncodeDecode(context, L"коммунизм");
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
        context.out << awl::FromACString(e.what()) << std::endl;

        auto * p_awl_e = dynamic_cast<const awl::Exception *>(&e);
        
        if (p_awl_e)
        {
            Print(context, *p_awl_e);
        }
    }
}
