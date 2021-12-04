/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/TestMap.h"
#include "Awl/Testing/TestAssert.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/LocalAttribute.h"

#include "Awl/StdConsole.h"
#include "Awl/IntRange.h"

#include <set>
#include <regex>

namespace awl::testing
{
    TestMap::TestMap() : nullOutput(&nullBuffer)
    {
        for (TestLink* p_link : GetTestChain())
        {
            if (!testMap.emplace(p_link->GetName(), p_link).second)
            {
                throw TestException(format() << _T("The test '" << p_link->GetName() << _T(" already exists.")));
            }
        }
    }

    void TestMap::Run(const TestContext& context, const Char* name)
    {
        auto i = testMap.find(name);

        if (i == testMap.end())
        {
            throw TestException(format() << _T("The test '" << name << _T(" does not exist.")));
        }

        InternalRun(i->second, context);
    }

    void TestMap::RunAll(const TestContext& context, const std::function<bool(const String&)>& filter)
    {
        for (auto& p : testMap)
        {
            const auto& test_name = p.first;

            if (filter(test_name))
            {
                InternalRun(p.second, context);
            }
        }
    }

    void TestMap::PrintNames(awl::ostream& out, const std::function<bool(const String&)>& filter) const
    {
        for (auto& p : testMap)
        {
            const auto& test_name = p.first;

            if (filter(test_name))
            {
                out << test_name << std::endl;
            }
        }
    }

    void TestMap::InternalRun(TestLink* p_test_link, const TestContext& context)
    {
        AWT_ATTRIBUTE(String, output, _T("failed"));
        AWT_ATTRIBUTE(size_t, loop, 0);

        context.out << p_test_link->GetName();

        size_t loop_count = loop;

        if (loop_count != 0)
        {
            context.out << _T(" Looping ") << loop_count << _T(" times.");
        }
        else
        {
            loop_count = 1;
        }

        context.out << _T("...") << std::endl;

        std::basic_ostream<Char>* p_out = nullptr;

        if (output == _T("all"))
        {
            p_out = &context.out;
        }
        else if (output == _T("failed"))
        {
            p_out = &lastOutput;
        }
        else if (output == _T("null"))
        {
            p_out = &nullOutput;
        }
        else
        {
            throw TestException(format() << _T("Not a valid 'output' parameter value: '") << output << _T("'."));
        }

        const TestContext temp_context{ *p_out, context.cancellation, context.ap };

        for (auto i : awl::make_count(loop_count))
        {
            static_cast<void>(i);
            p_test_link->Run(temp_context);
        }

        lastOutput.str(String());

        context.out << _T("OK") << std::endl;
    }
}
