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
                Option() : val(nullptr), usage(0)
                {
                }

                Option(const Char * v) : Option()
                {
                    val = v;
                }

                //A flag is an option that does not have a value.
                bool IsFlag() const
                {
                    return val != nullptr;
                }

                const Char * val;

                mutable size_t usage;
            };

            using OptionsMap = std::unordered_map<String, Option>;

            OptionsMap allOptions;
        };
    }
}
