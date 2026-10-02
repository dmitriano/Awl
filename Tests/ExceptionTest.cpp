/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Io/IoException.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"
#include "Awl/DataCast.h"

#include <locale.h>
#include <string>
#include <type_traits>

#if defined(_WIN32) || defined(__unix__) || defined(__APPLE__)
#include "Awl/Io/NativeException.h"
#endif

using namespace awl::testing;
using namespace awl::io;

static_assert(std::is_abstract_v<awl::Exception>);
static_assert(std::is_abstract_v<IoException>);
static_assert(!std::is_abstract_v<awl::GeneralException>);
static_assert(!std::is_abstract_v<EndOfFileException>);
static_assert(!std::is_abstract_v<CorruptionException>);
static_assert(!std::is_abstract_v<ReadFailException>);
static_assert(!std::is_abstract_v<WriteFailException>);
static_assert(!std::is_abstract_v<IoError>);
static_assert(!std::is_abstract_v<FieldNotFoundException>);
static_assert(!std::is_abstract_v<TypeMismatchException>);

static void Print(const TestContext & context, const awl::Exception & e)
{
    context.logger->debug(_T("{}"), e.message());
}

static void EncodeDecode(const TestContext &, const std::wstring sample)
{
    const std::string encoded = awl::encodeString(sample.c_str());

    const std::wstring decoded = awl::decodeString(encoded.c_str());

    AWL_ASSERT(decoded == sample);
}

AWL_TEST(DecodeString)
{
    setlocale(LC_ALL, "ru_RU.utf8");

    {
        const char * encoded = awl::char_cast(u8"z\u00df\u6c34\U0001f34c");

        context.logger->debug(_T("Decoded string: {}"), awl::fromACString(encoded));
    }

    EncodeDecode(context, L"");
}

AWL_TEST(ExceptionMessage)
{
    setlocale(LC_ALL, "en_US.utf8");

    const EndOfFileException end_of_file(5, 3);
    Print(context, end_of_file);
    AWL_ASSERT_EQUAL(_T("Requested 5 actually read 3 ."), end_of_file.message());

    const TestException e(_T("Test message."));
    Print(context, e);
    AWL_ASSERT_EQUAL(_T("Test message."), e.message());
    AWL_ASSERT_EQUAL(std::string("Test message."), std::string(e.what()));
}

AWL_TEST(ExceptionTryCatch)
{
    setlocale(LC_ALL, "en_US.utf8");

    bool caught = false;

    try
    {
        throw TestException(_T("Test message."));
    }
    catch (const std::exception & e)
    {
        caught = true;
        AWL_ASSERT_EQUAL(std::string("Test message."), std::string(e.what()));
        context.logger->debug(_T("{}"), awl::fromACString(e.what()));
    }

    AWL_ASSERT(caught);
}

AWL_TEST(ExceptionWhat)
{
    AWL_UNUSED_CONTEXT;

    const awl::GeneralException e(std::string("abc"));
    const awl::Exception& base = e;
    AWL_ASSERT_EQUAL(std::string("abc"), std::string(base.what()));
    AWL_ASSERT_EQUAL(_T("abc"), base.message());

    const awl::GeneralException empty{ std::string() };
    AWL_ASSERT_EQUAL(std::string(), std::string(empty.what()));
    AWL_ASSERT(empty.message().empty());
}

AWL_TEST(ExceptionWideMessage)
{
    AWL_UNUSED_CONTEXT;
    const std::wstring text = L"Wide message.";
    const awl::GeneralException e(text);
    AWL_ASSERT_EQUAL(std::string("Wide message."), std::string(e.what()));
    AWL_ASSERT_EQUAL(_T("Wide message."), e.message());

    const TestException derived(text);
    AWL_ASSERT_EQUAL(std::string(e.what()), std::string(derived.what()));
    AWL_ASSERT_EQUAL(e.message(), derived.message());

    const awl::GeneralException empty{ std::wstring() };
    AWL_ASSERT_EQUAL(std::string(), std::string(empty.what()));
    AWL_ASSERT(empty.message().empty());
}

AWL_TEST(ExceptionCopy)
{
    AWL_UNUSED_CONTEXT;

    const awl::GeneralException copied = []
    {
        const awl::GeneralException original(std::string(1024, 'x'));
        return awl::GeneralException(original);
    }();

    AWL_ASSERT_EQUAL(std::string(1024, 'x'), std::string(copied.what()));
    AWL_ASSERT_EQUAL(awl::String(1024, _T('x')), copied.message());
}

AWL_TEST(ExceptionIoMessages)
{
    AWL_UNUSED_CONTEXT;
    AWL_ASSERT_EQUAL(std::string("End of file."), std::string(EndOfFileException(5, 3).what()));
    AWL_ASSERT_EQUAL(std::string("The stream is corrupted."), std::string(CorruptionException(7).what()));
    AWL_ASSERT_EQUAL(_T("The stream is corrupted at 7 ."), CorruptionException(7).message());
    AWL_ASSERT_EQUAL(_T("The stream is corrupted ."), CorruptionException().message());
    AWL_ASSERT_EQUAL(_T("Read failed."), ReadFailException().message());
    AWL_ASSERT_EQUAL(_T("Write failed."), WriteFailException().message());

    const IoError error(_T("IO error."));
    AWL_ASSERT_EQUAL(std::string("IO error."), std::string(error.what()));
    AWL_ASSERT_EQUAL(_T("IO error."), error.message());

    const FieldNotFoundException field("abc");
    AWL_ASSERT_EQUAL(std::string("Field not found."), std::string(field.what()));
    AWL_ASSERT_EQUAL(_T("Field 'abc' not found. ."), field.message());

    const TypeMismatchException mismatch("abc", 2, 3);
    AWL_ASSERT_EQUAL(std::string("Type mismatch."), std::string(mismatch.what()));
    AWL_ASSERT_EQUAL(_T("Expected 'abc' type: 3 actually read type: 2 ."), mismatch.message());

#if defined(_WIN32) || defined(__unix__) || defined(__APPLE__)
    {
        const NativeException native(_T("Native error."), 7);
        AWL_ASSERT_EQUAL(std::string("Native error."), std::string(native.what()));
        AWL_ASSERT_EQUAL(_T("Native error. Error code: 7 ."), native.message());
    }
#endif
}
