#pragma once

#include <string>
#include <sstream>
#include <type_traits>
#include <algorithm>

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

    
    template <typename C>
    typename std::enable_if<!std::is_same<C, char>::value, std::basic_string<C>>::type FromAString(std::string src)
    {
        std::basic_string<C> dest(src.length(), ' ');

        std::copy(src.begin(), src.end(), dest.begin());

        return dest;
    }

    template <typename C>
    typename std::enable_if<std::is_same<C, char>::value, std::basic_string<C>>::type FromAString(std::string src)
    {
        return src;
    }
    
    template <typename C>
    typename std::enable_if<!std::is_same<C, char>::value, std::basic_string<C>>::type FromACString(const char * p_src)
    {
        std::basic_string<C> dest(strlen(p_src), ' ');

        auto p_cur = p_src;
        
        for (auto & ch : dest)
        {
            ch = *p_cur++;
        }

        return dest;
    }

    template <typename C>
    typename std::enable_if<std::is_same<C, char>::value, std::basic_string<C>>::type FromACString(const char * p_src)
    {
        return p_src;
    }
}
