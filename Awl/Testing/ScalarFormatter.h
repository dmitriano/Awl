/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

#include <type_traits>
namespace awl
{
    namespace testing
    {
        // used when C is specified as char
        template<typename C, typename T>
        typename std::enable_if<std::is_same<C, char>::value, std::string>::type toBasicString(T val)
        {
            return std::to_string(val);
        }

        // used when C is specified as wchar_t
        template<typename C, typename T>
        typename std::enable_if<std::is_same<C, wchar_t>::value, std::wstring>::type toBasicString(T val)
        {
            return std::to_wstring(val);
        }

        template<typename C>
        typename std::enable_if<std::is_same<C, char>::value || std::is_same<C, wchar_t>::value, std::basic_string<C>>::type toBasicString(bool)
        {
            return std::basic_string<C>{};
        }

        // used when C is specified as other types
        template<typename C, typename T>
        typename std::enable_if<!std::is_same<C, char>::value && !std::is_same<C, wchar_t>::value, std::basic_string<C>>::type toBasicString(T val);

        template <class C>
        class BasicScalarFormatter
        {
        public:

            using String = std::basic_string<C>;

            template<typename T>
            static String toString(T val)
            {
                return toBasicString<C, T>(val);
            }

            static void fromString(const String &, bool & val)
            {
                val = true;
            }

            static void fromString(const String & s, int & val)
            {
                val = std::stoi(s);
            }

            static void fromString(const String & s, unsigned int & val)
            {
                val = static_cast<unsigned int>(std::stoi(s));
            }

            static void fromString(const String & s, long & val)
            {
                val = std::stol(s);
            }

            static void fromString(const String & s, long long & val)
            {
                val = std::stoll(s);
            }

            static void fromString(const String & s, unsigned long & val)
            {
                val = std::stoul(s);
            }

            static void fromString(const String & s, unsigned long long & val)
            {
                val = std::stoull(s);
            }

            static void fromString(const String & s, float & val)
            {
                val = std::stof(s);
            }

            static void fromString(const String & s, double & val)
            {
                val = std::stod(s);
            }

            static void fromString(const String & s, long double & val)
            {
                val = std::stold(s);
            }
        };

        using ScalarFormatter = BasicScalarFormatter<Char>;
    }
}
