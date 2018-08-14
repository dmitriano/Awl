#pragma once

#include <string>
#include <sstream>

#ifdef _MSC_VER

#include <tchar.h>

#else

#ifndef TCHAR
#define TCHAR char
#endif

#if !defined(_T)	
#define _T(quoted_string) quoted_string
#endif

#endif

namespace awl
{
    typedef TCHAR Char;

    typedef std::basic_string<Char> String;

    typedef std::basic_istream<Char> istream;
    typedef std::basic_ostream<Char> ostream;
    
    typedef std::basic_ostringstream<Char> ostringstream;
    typedef std::basic_istringstream<Char> istringstream;

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

    template<typename T>
    inline String ToString(T val)
    {
        return ToBasicString<Char, T>(val);
    }

    inline void FromString(const String & s, int & val)
    {
        val = std::stoi(s);
    }

    inline void FromString(const String & s, long & val)
    {
        val = std::stol(s);
    }

    inline void FromString(const String & s, long long & val)
    {
        val = std::stoll(s);
    }

    inline void FromString(const String & s, unsigned long & val)
    {
        val = std::stoul(s);
    }

    inline void FromString(const String & s, unsigned long long & val)
    {
        val = std::stoull(s);
    }

    inline void FromString(const String & s, float & val)
    {
        val = std::stof(s);
    }

    inline void FromString(const String & s, double & val)
    {
        val = std::stod(s);
    }

    inline void FromString(const String & s, long double & val)
    {
        val = std::stold(s);
    }
}
