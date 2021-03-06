#pragma once

#include "Awl/Testing/ScalarFormatter.h"
#include "Awl/Testing/TestTypeTraits.h"

#include <iterator>

namespace awl
{
    namespace testing
    {
        template <typename C, typename T, typename Enable = void>
        class BasicFormatter;

        template <typename C, typename T>
        class BasicFormatter<C, T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
        {
        public:

            using String = std::basic_string<C>;

            static String ToString(T val)
            {
                return BasicScalarFormatter<C>::ToString(val);
            }

            static T FromString(const String & s)
            {
                T val;

                BasicScalarFormatter<C>::FromString(s, val);

                return val;
            }
        };

        template <typename C, typename T>
        class BasicFormatter<C, T, typename std::enable_if<is_string<C, T>::value>::type>
        {
        public:

            using String = std::basic_string<C>;

            static String ToString(T val)
            {
                return val;
            }

            static T FromString(String s)
            {
                return s;
            }
        };

        template <typename C, typename T>
        class BasicFormatter<C, T, typename std::enable_if<is_collection<T>::value &&
            (std::is_arithmetic<typename T::value_type>::value || is_string<C, typename T::value_type>::value)>::type>
        {
        public:

            using String = std::basic_string<C>;

            static String ToString(const T & val)
            {
                constexpr C separator = '\x20';

                std::basic_ostringstream<C> out;

                //This adds an extra separator at the end of the stream.
                //std::copy(val.begin(), val.end(), std::ostream_iterator<typename T::value_type, C>(out, GetSeparator<C>()));

                bool first = true;

                for (const auto e : val)
                {
                    if (first)
                    {
                        first = false;
                    }
                    else
                    {
                        out << separator;
                    }

                    out << e;
                }

                return out.str();
            }

            static T FromString(const String & s)
            {
                std::basic_istringstream<C> in(s);

                T val{ std::istream_iterator<typename T::value_type, C>(in), std::istream_iterator<typename T::value_type, C>() };

                return val;
            }
        };

        template<class T> using Formatter = BasicFormatter<Char, T>;
    }
}
