/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/StdConsole.h"
#include "Awl/StringFormat.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/TestException.h"

#include <regex>

namespace awl::testing
{
    CommandLineProvider::CommandLineProvider(int argc, CmdChar* argv[])
    {
        std::basic_regex<CmdChar> option_regex(StringConvertor<CmdChar>::convertFrom("--([[:alpha:]][_[:alpha:][:digit:]]+)(=(.*))?"),
            std::regex_constants::icase);

        std::match_results<const CmdChar*> match;

        int i = 1;

        while (i < argc)
        {
            const CmdChar* val = argv[i++];

            if (std::regex_match(val, match, option_regex) && match.size() == 4)
            {
                CmdString name = match[1].str();
                const CmdChar* option_value = nullptr;

                if (match[3].matched)
                {
                    option_value = val + match.position(3);
                }

                auto result = allOptions.emplace(StringConvertor<char>::convertFrom(name.c_str()), Option{ option_value });

                if (!result.second)
                {
                    throw TestException(std::format(_T("Duplicated option '{}'."), name));
                }
            }
            else
            {
                const CmdString arg = val;

                if (arg.starts_with(StringConvertor<CmdChar>::convertFrom("--")))
                {
                    throw TestException(std::format(_T("Invalid option syntax '{}'. Use '--name' for flags or '--name=value' for attributes."),
                        StringConvertor<Char>::convertFrom(val)));
                }

                throw TestException(std::format(_T("An option name starting with '--' expected near {}"),
                    StringConvertor<Char>::convertFrom(val)));
            }
        }
    }

    bool CommandLineProvider::tryFind(const char* name, CmdString& val) const
    {
        auto i = allOptions.find(name);

        if (i != allOptions.end())
        {
            ++(i->second.usage);

            const CmdChar* raw_val = i->second.val;

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
