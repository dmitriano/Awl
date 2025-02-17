/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/StringFormat.h"
#include "Awl/Testing/Formatter.h"
#include "Awl/Testing/AttributeProvider.h"

#include <unordered_map>
#include <ranges>

namespace awl::testing
{

#ifdef AWL_ANSI_CMD_CHAR
    using CmdChar = char;
#else
    using CmdChar = Char;
#endif;

    using CmdString = std::basic_string<CmdChar>;

    class CommandLineProvider
    {
    public:

        CommandLineProvider(int argc, CmdChar* argv[]);

        auto GetUnusedOptions() const
        {
            return allOptions | std::views::filter([](const auto& pair) -> bool { return pair.second.usage == 0; }) |
                std::views::keys;
        }

        template <class T>
        bool TryGet(const char* name, T& val)
        {
            CmdString s;

            if (TryFind(name, s))
            {
                if constexpr (BasicFormatter<CmdChar, T>::value)
                {
                    val = BasicFormatter<CmdChar, T>::FromString(s);

                    return true;
                }
                else
                {
                    // The attribute is found, but we can't parse it.
                    throw GeneralException(awl::format() << "Attribute '" << name << "' is not supported by CommandLineProvider.");
                }
            }

            return false;
        }

    private:

        bool TryFind(const char* name, CmdString& val) const;

        struct Option
        {
            Option() : val(nullptr), usage(0)
            {
            }

            Option(const CmdChar* v) : Option()
            {
                val = v;
            }

            //A flag is an option that does not have a value.
            bool IsFlag() const
            {
                return val != nullptr;
            }

            const CmdChar* val;

            mutable size_t usage;
        };

        using OptionsMap = std::unordered_map<std::string, Option>;

        OptionsMap allOptions;
    };

    static_assert(attribute_provider<CommandLineProvider>);
}
