/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstring>
#include <string>
#include <string_view>
#include <sstream>
#include <type_traits>
#include <algorithm>
#include <wchar.h>
#include <cassert>
#include <iterator>

#include "Awl/FixedString.h"

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
    using Char = TCHAR;

    template<std::size_t N>
    auto text(const char(&arr)[N])
    {
        return fixed_string<Char, N - 1>::from_ascii(arr);
    }

    template<typename CharT, std::size_t N>
    std::basic_ostream<CharT>& operator << (std::basic_ostream<CharT>& out, const fixed_string<Char, N>& val)
    {
        //Why it does not compile with std::basic_string_view<const CharT> ?
        out << static_cast<std::basic_string<CharT>>(val);

        return out;
    }

    using String = std::basic_string<Char>;

    static_assert(std::is_same<String::value_type, TCHAR>::value, "String::value_type is not TCHAR.");

    using istream = std::basic_istream<Char>;
    using ostream = std::basic_ostream<Char>;

    template <class T>
    using ostream_iterator = std::ostream_iterator<T, Char>;
    
    using ostringstream = std::basic_ostringstream<Char>;
    using istringstream = std::basic_istringstream<Char>;

    inline auto StrLen(const char * s)
    {
        return std::strlen(s);
    }

    inline auto StrLen(const wchar_t * s)
    {
        return std::wcslen(s);
    }

    inline auto StrCmp(const char * left, const char * right)
    {
        return std::strcmp(left, right);
    }

    inline auto StrCmp(const wchar_t * left, const wchar_t * right)
    {
        return std::wcscmp(left, right);
    }

    //C-string comparator.
    template <typename Ch>
    struct CStringLess
    {
        bool operator()(const Ch * left, const Ch * right) const
        {
            return StrCmp(left, right) < 0;
        }
    };

#ifdef _MSC_VER

    class string_encoding_error : public std::exception
    {
    public:

        string_encoding_error(size_t e) : error(e)
        {
        }

        const char * what() const throw() override
        {
            return typeid(*this).name();
        }

        size_t error_number() const
        {
            return error;
        }

    private:

        const size_t error;
    };
    
    inline std::string EncodeString(const wchar_t* wstr)
    {
        std::mbstate_t state = std::mbstate_t();

        //The length includes the teminating zero.
        size_t len;
        size_t error = wcsrtombs_s(&len, nullptr, 0, &wstr, 0, &state);

        if (error != 0)
        {
            throw string_encoding_error(error);
        }

        std::string mbstr(len - 1, ' ');
        size_t ret_val;
        error = wcsrtombs_s(&ret_val, &mbstr[0], mbstr.size() + 1, &wstr, len, &state);

        if (error != 0 || ret_val != len)
        {
            throw string_encoding_error(error);
        }

        return mbstr;
    }

    inline std::wstring DecodeString(const char* mbstr)
    {
        std::mbstate_t state = std::mbstate_t();
        
        //The length includes the teminating zero.
        size_t len;
        size_t error = mbsrtowcs_s(&len, nullptr, 0, &mbstr, 0, &state);
        
        if (error != 0)
        {
            throw string_encoding_error(error);
        }

        std::wstring wstr(len - 1, ' ');
        size_t ret_val;
        error = mbsrtowcs_s(&ret_val, &wstr[0], wstr.size() + 1,  &mbstr, len, &state);
        
        if (error != 0 || ret_val != len)
        {
            throw string_encoding_error(error);
        }

        return wstr;
    }

#else

    inline std::string EncodeString(const wchar_t* wstr)
    {
        std::mbstate_t state = std::mbstate_t();
        //This length does not include the terminating zero.
        std::size_t len = std::wcsrtombs(nullptr, &wstr, 0, &state);
        std::string mbstr(len, ' ');
        std::wcsrtombs(&mbstr[0], &wstr, len + 1, &state);
        return mbstr;
    }

    inline std::wstring DecodeString(const char* mbstr)
    {
        std::mbstate_t state = std::mbstate_t();
        //This length does not include the terminating zero.
        std::size_t len = std::mbsrtowcs(nullptr, &mbstr, 0, &state);
        std::wstring wstr(len, ' ');
        std::mbsrtowcs(&wstr[0], &mbstr, len + 1, &state);
        return wstr;
    }

#endif

    //'if constexpr' will only conditionally compile a branch within a template. Outside of a template,
    //all branches will be compiled and must be well formed.
    template <class Ch>
    struct StringConvertor
    {
        static std::basic_string<Ch> Decode(const char * p_src)
        {
            if constexpr (std::is_same_v<Ch, char>)
            {
                return p_src;
            }
            else
            {
                return DecodeString(p_src);
            }
        }

        static std::string Encode(const Ch * p_src)
        {
            if constexpr (std::is_same_v<Ch, char>)
            {
                return p_src;
            }
            else
            {
                return EncodeString(p_src);
            }
        }
    };
    
    inline String FromACString(const char * p_src)
    {
        return StringConvertor<Char>::Decode(p_src);
    }

    inline String FromAString(const std::string & src)
    {
        return StringConvertor<Char>::Decode(src.c_str());
    }

    inline std::string ToAString(const String & src)
    {
        return StringConvertor<Char>::Encode(src.c_str());
    }
}
