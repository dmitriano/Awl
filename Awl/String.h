#pragma once

#include <string>
#include <sstream>
#include <type_traits>
#include <algorithm>
#include <wchar.h>
#include <assert.h>

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

#ifdef _MSC_VER

    class decode_error : std::exception
    {
    public:

        decode_error(std::string s) : src(s)
        {
        }

        const char * what() const throw() override
        {
            return src.c_str();
        }

    private:

        std::string src;
    };
    
    inline std::wstring DecodeString(const char* mbstr)
    {
        std::mbstate_t state = std::mbstate_t();
        
        size_t len;
        size_t success = mbsrtowcs_s(&len, nullptr, 0, &mbstr, 0, &state);
        
        if (success != 0)
        {
            throw decode_error(mbstr);
        }

        std::wstring wstr(len, ' ');
        size_t ret_val;
        success = mbsrtowcs_s(&ret_val, &wstr[0], len,  &mbstr, wstr.size(), &state);
        
        if (success != 0 || ret_val != len)
        {
            throw decode_error(mbstr);
        }

        return wstr;
    }

#else

    inline std::wstring DecodeString(const char* mbstr)
    {
        std::mbstate_t state = std::mbstate_t();
        std::size_t len = 1 + std::mbsrtowcs(nullptr, &mbstr, 0, &state);
        std::wstring wstr(len, ' ');
        std::mbsrtowcs(&wstr[0], &mbstr, wstr.size(), &state);
        return wstr;
    }

#endif
    
    template <typename C>
    inline typename std::enable_if<std::is_same<C, wchar_t>::value, std::basic_string<C>>::type FromACStringHelper(const char * p_src)
    {
        return DecodeString(p_src);
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

#if AWL_CPPSTD >= 17

    inline String FromAString(std::string src)
    {
        if constexpr (std::is_same_v<Char, char>)
        {
            return std::forward<std::string>(src);
        }

        return FromACString(src.c_str());
    }

#else

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

#endif
}
