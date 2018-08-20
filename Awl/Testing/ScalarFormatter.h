#pragma once

#include "Awl/String.h"

#include <type_traits>

namespace awl
{
    namespace testing
    {
        // used when C is specified as char
        template<typename C, typename T>
        inline typename std::enable_if<std::is_same<C, char>::value, std::string>::type ToBasicString(T val)
        {
            return std::to_string(val);
        }

        // used when C is specified as wchar_t
        template<typename C, typename T>
        inline typename std::enable_if<std::is_same<C, wchar_t>::value, std::wstring>::type ToBasicString(T val)
        {
            return std::to_wstring(val);
        }

        // used when C is specified as other types
        template<typename C, typename T>
        typename std::enable_if<!std::is_same<C, char>::value && !std::is_same<C, wchar_t>::value, std::basic_string<C>>::type ToBasicString(T val);

        template <class C>
        class BasicScalarFormatter
        {
        public:

            typedef std::basic_string<C> String;

            template<typename T>
            static String ToString(T val)
            {
                return ToBasicString<C, T>(val);
            }

            template<>
            static String ToString(bool val)
            {
                return String{};
            }

            static void FromString(const String & s, bool & val)
            {
                val = true;
            }

            static void FromString(const String & s, int & val)
            {
                val = std::stoi(s);
            }

            static void FromString(const String & s, long & val)
            {
                val = std::stol(s);
            }

            static void FromString(const String & s, long long & val)
            {
                val = std::stoll(s);
            }

            static void FromString(const String & s, unsigned long & val)
            {
                val = std::stoul(s);
            }

            static void FromString(const String & s, unsigned long long & val)
            {
                val = std::stoull(s);
            }

            static void FromString(const String & s, float & val)
            {
                val = std::stof(s);
            }

            static void FromString(const String & s, double & val)
            {
                val = std::stod(s);
            }

            static void FromString(const String & s, long double & val)
            {
                val = std::stold(s);
            }
        };

        typedef BasicScalarFormatter<Char> ScalarFormatter;
    }
}
