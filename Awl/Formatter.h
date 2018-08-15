#pragma once

#include "Awl/ScalarFormatter.h"
#include "Awl/TypeTraits.h"

namespace awl
{
    template <typename C, typename T, typename Enable = void>
    class BasicFormatter;

    template <typename C, typename T>
    class BasicFormatter<C, T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
    { 
    public:

        typedef std::basic_string<C> String;

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
    class BasicFormatter<C, T, typename std::enable_if<is_collection<T>::value && 
        (std::is_arithmetic<typename T::value_type>::value || is_string<typename T::value_type>::value)>::type>
    {
    public:

        typedef std::basic_string<C> String;

        static String ToString(T val)
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

            T val { std::istream_iterator<typename T::value_type, C>(in), std::istream_iterator<typename T::value_type, C>() };

            return val;
        }
    };

    template<class T> using Formatter = BasicFormatter<Char, T>;
}
