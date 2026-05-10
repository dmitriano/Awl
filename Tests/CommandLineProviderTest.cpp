/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/UnitTest.h"

#include <vector>

using namespace awl::testing;

#ifdef AWL_ANSI_CMD_CHAR
    #define AWL_CMD_TEXT(s) s
#else
    #define AWL_CMD_TEXT(s) _T(s)
#endif

namespace
{
    CommandLineProvider makeCommandLineProvider(std::initializer_list<const CmdChar*> args)
    {
        std::vector<CmdChar*> argv;

        for (const CmdChar* arg : args)
        {
            argv.push_back(const_cast<CmdChar*>(arg));
        }

        return CommandLineProvider(static_cast<int>(argv.size()), argv.data());
    }
}

AWL_TEST(CommandLineProvider_EqualsSyntax)
{
    AWL_UNUSED_CONTEXT;

    CommandLineProvider provider = makeCommandLineProvider({
        AWL_CMD_TEXT("AwlTest"),
        AWL_CMD_TEXT("--run=ILogger_Test"),
        AWL_CMD_TEXT("--output=all"),
        AWL_CMD_TEXT("--thread_count=10"),
        AWL_CMD_TEXT("--empty="),
        AWL_CMD_TEXT("--flag")
    });

    std::string run;
    std::string output;
    std::string empty;
    size_t thread_count = 0;
    bool flag = false;

    AWL_ASSERT(provider.tryGet("run", run));
    AWL_ASSERT(provider.tryGet("output", output));
    AWL_ASSERT(provider.tryGet("thread_count", thread_count));
    AWL_ASSERT(provider.tryGet("empty", empty));
    AWL_ASSERT(provider.tryGet("flag", flag));

    AWL_ASSERT(run == "ILogger_Test");
    AWL_ASSERT(output == "all");
    AWL_ASSERT(thread_count == 10);
    AWL_ASSERT(empty.empty());
    AWL_ASSERT(flag);
}

AWL_TEST(CommandLineProvider_RejectsSpaceSeparatedValue)
{
    AWL_UNUSED_CONTEXT;

    Assert::throws<TestException>([]()
    {
        makeCommandLineProvider({
            AWL_CMD_TEXT("AwlTest"),
            AWL_CMD_TEXT("--run"),
            AWL_CMD_TEXT("ILogger_Test")
        });
    });
}

AWL_TEST(CommandLineProvider_RejectsDuplicateOption)
{
    AWL_UNUSED_CONTEXT;

    Assert::throws<TestException>([]()
    {
        makeCommandLineProvider({
            AWL_CMD_TEXT("AwlTest"),
            AWL_CMD_TEXT("--run=ILogger_Test"),
            AWL_CMD_TEXT("--run=CompositeLogger_Test")
        });
    });
}

#undef AWL_CMD_TEXT
