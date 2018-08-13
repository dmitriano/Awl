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
