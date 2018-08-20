#pragma once

#include "Awl/String.h"
#include "Awl/Testing/AttributeProvider.h"

#include <unordered_map>

namespace awl
{
    namespace testing
    {
        class CommandLineProvider : public AttributeProvider
        {
        public:

            CommandLineProvider(int argc, Char * argv[]);

            ~CommandLineProvider();

            bool TryFind(const String & name, String & val) const override;

        private:

            struct Option
            {
                Option() : usage(0)
                {
                }
                
                Option(const String & v) : Option()
                {
                    val = v;
                }

                //A flag is an option that does not have a value.
                bool IsFlag() const
                {
                    return val.empty();
                }

                String val;

                mutable size_t usage;
            };
            
            typedef std::unordered_map<String, Option> OptionsMap;

            OptionsMap allOptions;
        };
    }
}
