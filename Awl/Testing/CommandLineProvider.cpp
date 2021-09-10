/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/StdConsole.h"
#include "Awl/StringFormat.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/TestException.h"

#include <regex>

namespace awl
{
    namespace testing
    {
        CommandLineProvider::CommandLineProvider(int argc, Char * argv[])
        {
            std::basic_regex<Char> option_regex(_T("--([[:alpha:]][_[:alpha:][:digit:]]+)"));

            std::match_results<const Char *> match;

            OptionsMap::iterator current_option = allOptions.end();

            int i = 1;

            while (i < argc)
            {
                const Char * val = argv[i++];

                if (std::regex_match(val, match, option_regex) && match.size() == 2)
                {
                    String name = match[1].str();

                    auto result = allOptions.emplace(name, Option{});

                    if (!result.second)
                    {
                        throw TestException(format() << _T("Duplicated option '" << name << _T("'.")));
                    }

                    current_option = result.first;
                }
                else
                {
                    if (current_option != allOptions.end())
                    {
                        current_option->second = val;

                        current_option = allOptions.end();
                    }
                    else
                    {
                        throw TestException(format() << _T("An option name starting with '--' expected near ") << val);
                    }
                }
            }
        }

        CommandLineProvider::~CommandLineProvider()
        {
            for (const auto & p : allOptions)
            {
                if (p.second.usage == 0)
                {
                    cout() << _T("Unused option '" << p.first << _T("'")) << std::endl;
                }
            }
        }

        bool CommandLineProvider::TryFind(const String & name, String & val) const
        {
            auto i = allOptions.find(name);

            if (i != allOptions.end())
            {
                ++(i->second.usage);

                const Char * raw_val = i->second.val;

                if (raw_val == nullptr)
                {
                    val.clear();
                }
                else
                {
                    val = i->second.val;
                }

                return true;
            }

            return false;
        }
    }
}
