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

        protected:

            bool TryFind(const String & name, String & val) const override;

        private:

            struct Option
            {
                Option(const String & v) : val(v), usage(0)
                {
                }

                String val;

                mutable size_t usage;
            };
            
            typedef std::unordered_map<String, Option> OptionsMap;

            OptionsMap allOptions;
        };
    }
}
