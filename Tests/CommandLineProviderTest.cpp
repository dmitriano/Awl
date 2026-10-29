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
        AWL_CMD_TEXT("--signed_short=-32768"),
        AWL_CMD_TEXT("--unsigned_short=65535"),
        AWL_CMD_TEXT("--empty="),
        AWL_CMD_TEXT("--flag")
    });

    std::string run;
    std::string output;
    std::string empty;
    size_t thread_count = 0;
    signed short signed_short = 0;
    unsigned short unsigned_short = 0;
    bool flag = false;

    AWL_ASSERT(provider.tryGet("run", run));
    AWL_ASSERT(provider.tryGet("output", output));
    AWL_ASSERT(provider.tryGet("thread_count", thread_count));
    AWL_ASSERT(provider.tryGet("signed_short", signed_short));
    AWL_ASSERT(provider.tryGet("unsigned_short", unsigned_short));
    AWL_ASSERT(provider.tryGet("empty", empty));
    AWL_ASSERT(provider.tryGet("flag", flag));

    AWL_ASSERT(run == "ILogger_Test");
    AWL_ASSERT(output == "all");
    AWL_ASSERT(thread_count == 10);
    AWL_ASSERT_EQUAL(-32768, signed_short);
    AWL_ASSERT_EQUAL(65535, unsigned_short);
    AWL_ASSERT(empty.empty());
    AWL_ASSERT(flag);
}

AWL_TEST(CommandLineProvider_Duration)
{
    AWL_UNUSED_CONTEXT;
    using namespace std::chrono_literals;

    CommandLineProvider provider = makeCommandLineProvider({
        AWL_CMD_TEXT("AwlTest"),
        AWL_CMD_TEXT("--read_timeout=10s"),
        AWL_CMD_TEXT("--request_timeout=3m"),
        AWL_CMD_TEXT("--precise=2d.3h.4m.5s.006"),
        AWL_CMD_TEXT("--optional_timeout=0.1"),
        AWL_CMD_TEXT("--no_timeout=null")
    });

    std::chrono::milliseconds read_timeout{};
    std::chrono::milliseconds request_timeout{};
    std::chrono::milliseconds precise{};
    std::optional<std::chrono::milliseconds> optional_timeout;
    std::optional<std::chrono::milliseconds> no_timeout = 1s;
    AWL_ASSERT(provider.tryGet("read_timeout", read_timeout));
    AWL_ASSERT(provider.tryGet("request_timeout", request_timeout));
    AWL_ASSERT(provider.tryGet("precise", precise));
    AWL_ASSERT(provider.tryGet("optional_timeout", optional_timeout));
    AWL_ASSERT(provider.tryGet("no_timeout", no_timeout));
    AWL_ASSERT(read_timeout == 10s);
    AWL_ASSERT(request_timeout == 3min);
    AWL_ASSERT(precise == std::chrono::days(2) + 3h + 4min + 5s + 6ms);
    AWL_ASSERT(optional_timeout == 100ms);
    AWL_ASSERT_FALSE(no_timeout.has_value());

    // Both console character types use the same exact duration representation.
    using Narrow = BasicFormatter<char, std::chrono::milliseconds>;
    using Wide = BasicFormatter<wchar_t, std::chrono::milliseconds>;
    AWL_ASSERT_EQUAL(std::string("2d.3h.4m.5s.006"), Narrow::toString(precise));
    AWL_ASSERT_EQUAL(std::wstring(L"2d.3h.4m.5s.006"), Wide::toString(precise));
    AWL_ASSERT(Narrow::fromString(Narrow::toString(precise)) == precise);
    AWL_ASSERT(Wide::fromString(Wide::toString(precise)) == precise);
    AWL_ASSERT(Narrow::fromString("-1s") == -1s);
}

AWL_TEST(CommandLineProvider_DurationErrors)
{
    AWL_UNUSED_CONTEXT;

    const auto check_rejected = [](const auto action)
    {
        bool caught = false;

        try
        {
            action();
        }
        catch (const awl::GeneralException&)
        {
            caught = true;
        }

        AWL_ASSERT(caught);
    };

    for (const CmdChar* option : {
        AWL_CMD_TEXT("--duration=10000"),
        AWL_CMD_TEXT("--duration=bad"),
        AWL_CMD_TEXT("--duration=0.000000001"),
        AWL_CMD_TEXT("--duration=999999999999999999999s") })
    {
        CommandLineProvider provider = makeCommandLineProvider({ AWL_CMD_TEXT("AwlTest"), option });
        std::chrono::milliseconds duration{};
        check_rejected([&]
        {
            provider.tryGet("duration", duration);
        });
    }

    check_rejected([]
    {
        BasicFormatter<wchar_t, std::chrono::milliseconds>::fromString(L"bad");
    });
}

AWL_TEST(CommandLineProvider_RejectsSpaceSeparatedValue)
{
    AWL_UNUSED_CONTEXT;

    Assert::throws<CommandLineException>([]()
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

    Assert::throws<CommandLineException>([]()
    {
        makeCommandLineProvider({
            AWL_CMD_TEXT("AwlTest"),
            AWL_CMD_TEXT("--run=ILogger_Test"),
            AWL_CMD_TEXT("--run=CompositeLogger_Test")
        });
    });
}

#undef AWL_CMD_TEXT
