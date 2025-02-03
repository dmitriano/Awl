/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/Testing/Formatter.h"
#include "Awl/Testing/AttributeProvider.h"

#include <unordered_map>

namespace awl
{
    namespace testing
    {
        class CommandLineProvider
        {
        public:

            CommandLineProvider(int argc, Char * argv[]);

            void PrintUnusedOptions();

            template <class T>
            bool TryGet(const char* name, T& val)
            {
                String s;

                if (TryFind(name, s))
                {
                    val = Formatter<T>::FromString(s);

                    return true;
                }

                return false;
            }

        private:

            bool TryFind(const char* name, String& val) const;

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

            using OptionsMap = std::unordered_map<std::string, Option>;

            OptionsMap allOptions;
        };
    }
}
