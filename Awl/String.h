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

static_assert(sizeof(_T(" ")[0]) == sizeof(TCHAR), "Wrong _T macro definition.");

namespace awl
{
    typedef TCHAR Char;

    typedef std::basic_string<Char> String;

    static_assert(std::is_same<String::value_type, TCHAR>::value, "String::value_type is not TCHAR.");

    typedef std::basic_istream<Char> istream;
    typedef std::basic_ostream<Char> ostream;
    
    typedef std::basic_ostringstream<Char> ostringstream;
    typedef std::basic_istringstream<Char> istringstream;

    
    template <typename C>
    typename std::enable_if<!std::is_same<C, char>::value, std::basic_string<C>>::type FromACStringHelper(const char * p_src)
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
    inline typename std::enable_if<std::is_same<C, char>::value, std::basic_string<C>>::type FromACStringHelper(const char * p_src)
    {
        return p_src;
    }

    inline String FromACString(const char * p_src)
    {
        return FromACStringHelper<Char>(p_src);
    }

    template <typename C>
    inline typename std::enable_if<!std::is_same<C, char>::value, std::basic_string<C>>::type FromAStringHelper(std::string src)
    {
        return FromACString(src.c_str());
    }

    template <typename C>
    inline typename std::enable_if<std::is_same<C, char>::value, std::basic_string<C>>::type FromAStringHelper(std::string src)
    {
        return std::forward<std::string>(src);
    }

    inline String FromAString(std::string src)
    {
        return FromAStringHelper<Char>(src);
    }
}
