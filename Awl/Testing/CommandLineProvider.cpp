#pragma once

#include "Awl/StdConsole.h"
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

            std::match_results<String::const_iterator> match;

            int i = 1;

            while (i < argc)
            {
                String option = argv[i];
                
                if (std::regex_match(option, match, option_regex))
                {
                    if (match.size() == 2)
                    {
                        String name = match[1].str();

                        ++i;

                        if (i < argc)
                        {
                            String val = argv[i];
                            
                            allOptions.emplace(name, val);

                            ++i;
                        }
                        else
                        {
                            ostringstream out;

                            out << _T("The value of '") << name << _T("' expected.");

                            throw TestException(out.str());
                        }
                    }
                }
                else
                {
                    ostringstream out;

                    out << _T("An option name starting with '--' expected near ") << option;

                    throw TestException(out.str());
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

                val = i->second.val;

                return true;
            }

            return false;
        }
    }
}
