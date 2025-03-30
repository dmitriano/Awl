/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef AWL_QT
    #include "QtExtras/StringConversion.h"
#endif

#include "Awl/Testing/ScalarFormatter.h"
#include "Awl/TypeTraits.h"
#include "Awl/Inserter.h"
#include "Awl/EnumTraits.h"

namespace awl::testing
{
    template <typename C, typename T>
    class BasicFormatter : public std::false_type {};

    template <typename C, typename T> requires std::is_arithmetic_v<T>
    class BasicFormatter<C, T> : public std::true_type
    {
    public:

        using String = std::basic_string<C>;

        static String ToString(T val)
        {
            return BasicScalarFormatter<C>::ToString(val);
        }

        static T FromString(const String& s)
        {
            T val;

            BasicScalarFormatter<C>::FromString(s, val);

            return val;
        }
    };

    template <typename C, typename T> requires awl::is_sequential_enum<T>
    class BasicFormatter<C, T> : public std::true_type
    {
    public:

        using String = std::basic_string<C>;

        static String ToString(T val)
        {
            return awl::FromAString(awl::enum_to_string(val));
        }

        static T FromString(const String& s)
        {
            return awl::enum_from_string<T>(awl::ToAString(s));
        }
    };

    template <typename C, typename T> requires is_specialization_v<T, std::basic_string>
    class BasicFormatter<C, T> : public std::true_type
    {
    public:

        using String = std::basic_string<C>;

        static String ToString(T val)
        {
            return StringConvertor<typename String::value_type>::ConvertFrom(val.c_str());
        }

        static T FromString(String s)
        {
            return StringConvertor<typename T::value_type>::ConvertFrom(s.c_str());
        }
    };

    template <typename C, typename T> requires inserter_defined<T> &&
        (std::is_arithmetic<typename T::value_type>::value || is_string<typename T::value_type>)
        class BasicFormatter<C, T> : public std::true_type
    {
    public:

        using String = std::basic_string<C>;

        static String ToString(const T& val)
        {
            constexpr C separator = '\x20';

            std::basic_ostringstream<C> out;

            //This adds an extra separator at the end of the stream.
            //std::copy(val.begin(), val.end(), std::ostream_iterator<typename T::value_type, C>(out, GetSeparator<C>()));

            bool first = true;

            for (const auto& e : val)
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

        static T FromString(const String& s)
        {
            std::basic_istringstream<C> in(s);

            T val{ std::istream_iterator<typename T::value_type, C>(in), std::istream_iterator<typename T::value_type, C>() };

            return val;
        }
    };

#ifdef AWL_QT

    template <typename C>
    class BasicFormatter<C, QString> : public std::true_type
    {
    public:

        using String = std::basic_string<C>;

        static String ToString(QString val)
        {
            return FromQString<typename String::value_type>(val);
        }

        static QString FromString(String s)
        {
            return ToQString(s);
        }
    };

#endif

    template <class T>
    using Formatter = BasicFormatter<Char, T>;
}
