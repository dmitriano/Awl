/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/StringFormat.h"
#include "Awl/Exception.h"
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
#endif

    using CmdString = std::basic_string<CmdChar>;

    class CommandLineProvider
    {
    public:

        CommandLineProvider(int argc, CmdChar* argv[]);

        auto getUnusedOptions() const
        {
            return allOptions | std::views::filter([](const auto& pair) -> bool { return pair.second.usage == 0; }) |
                std::views::keys;
        }

        template <class T>
        bool tryGet(const char* name, T& val)
        {
            CmdString s;

            if (tryFind(name, s))
            {
                if constexpr (BasicFormatter<CmdChar, T>::value)
                {
                    val = BasicFormatter<CmdChar, T>::fromString(s);

                    return true;
                }
                else
                {
                    // The attribute is found, but we can't parse it.
                    throw GeneralException(std::format(_T("Attribute '{}' is not supported by CommandLineProvider."), awl::fromACString(name)));
                }
            }

            return false;
        }

        template <class T>
        void set(const char* name, const T& val)
        {
            static_cast<void>(name);
            static_cast<void>(val);
        }

        // Command line attributes applies to all the tests.
        void clear() {}

    private:

        bool tryFind(const char* name, CmdString& val) const;

        struct Option
        {
            Option() : val(nullptr), usage(0)
            {}

            Option(const CmdChar* val_ptr) : Option()
            {
                val = val_ptr;
            }

            //A flag is an option that does not have a value.
            bool isFlag() const
            {
                return val == nullptr;
            }

            const CmdChar* val;

            mutable size_t usage;
        };

        using OptionsMap = std::unordered_map<std::string, Option>;

        OptionsMap allOptions;
    };

    static_assert(attribute_provider<CommandLineProvider>);
}
