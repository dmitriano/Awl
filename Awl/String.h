/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/FixedString.h"
#include "Awl/CppStd/StringView.h"

#ifdef AWL_QT
    #include <QString>
#endif //AWL_QT

#ifdef AWL_BOOST
#include "BoostExtras/MultiprecisionDecimalData.h"
#endif

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <algorithm>
#include <wchar.h>
#include <cassert>
#include <iterator>
#include <optional>

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

    template<typename CharT, std::size_t N>
    auto text_literal(const char(&arr)[N])
    {
        return fixed_string<CharT, N - 1>::from_ascii(arr);
    }

    template<std::size_t N>
    auto text(const char(&arr)[N])
    {
        return text_literal<Char>(arr);
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

    inline auto StrLen(const char* s)
    {
        return std::strlen(s);
    }

    inline auto StrLen(const wchar_t* s)
    {
        return std::wcslen(s);
    }

    inline auto StrCmp(const char* left, const char* right)
    {
        return std::strcmp(left, right);
    }

    inline auto StrCmp(const wchar_t* left, const wchar_t* right)
    {
        return std::wcscmp(left, right);
    }

    //C-string comparator.
    template <typename Ch>
    struct CStringLess
    {
        bool operator()(const Ch* left, const Ch* right) const
        {
            //Returns negative value if left appears before right in lexicographical order.
            return StrCmp(left, right) < 0;
        }
    };

    template <typename Ch>
    struct CStringEqual
    {
        bool operator()(const Ch* left, const Ch* right) const
        {
            return StrCmp(left, right) == 0;
        }
    };

    template <typename Ch>
    int StrCmpI(const Ch* left, const Ch* right)
    {
        const Ch* l = left;
        const Ch* r = right;

        while (*l != 0 && *r != 0)
        {
            auto diff = std::tolower(*l++) - std::tolower(*r++);

            if (diff != 0)
            {
                return diff;
            }
        }

        if (*l == 0 && *r == 0)
        {
            return 0;
        }

        return *r != 0 ? -1 : 1;
    }

    template <typename Ch>
    struct CStringInsensitiveLess
    {
        bool operator()(const Ch* left, const Ch* right) const
        {
            return StrCmpI(left, right) < 0;
        }
    };

    template <typename Ch>
    struct CStringInsensitiveEqual
    {
        bool operator()(const Ch* left, const Ch* right) const
        {
            return StrCmpI(left, right) == 0;
        }
    };

    template <typename Ch>
    struct StringEqual
    {
        bool operator()(const std::basic_string<Ch>& str1, const std::basic_string<Ch>& str2) const
        {
            return str1 == str2;
        }
    };

    template <typename Ch>
    struct CharInsensitiveEqual
    {
        bool operator() (Ch c1, Ch c2) const
        {
            return std::tolower(c1) == std::tolower(c2);
        }
    };

    template <typename Ch>
    struct StringInsensitiveEqual
    {
        bool operator()(const std::basic_string<Ch>& str1, const std::basic_string<Ch>& str2) const
        {
            return ((str1.size() == str2.size()) &&
                std::equal(str1.begin(), str1.end(), str2.begin(), CharInsensitiveEqual<Ch>()));
        }
    };

#ifdef _MSC_VER

    class string_encoding_error : public std::exception
    {
    public:

        string_encoding_error(size_t e) : error(e)
        {
        }

        const char* what() const throw() override
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
        error = mbsrtowcs_s(&ret_val, &wstr[0], wstr.size() + 1, &mbstr, len, &state);

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
        static std::basic_string<Ch> ConvertFrom(const char* p_src)
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

        static std::basic_string<Ch> ConvertFrom(const wchar_t* p_src)
        {
            if constexpr (std::is_same_v<Ch, wchar_t>)
            {
                return p_src;
            }
            else
            {
                return EncodeString(p_src);
            }
        }
    };

    inline String FromACString(const char* p_src)
    {
        return StringConvertor<Char>::ConvertFrom(p_src);
    }

    inline String FromAString(const std::string& src)
    {
        return StringConvertor<Char>::ConvertFrom(src.c_str());
    }

    inline std::string ToAString(const String& src)
    {
        return StringConvertor<char>::ConvertFrom(src.c_str());
    }

    inline std::wstring ToWString(const String& src)
    {
        return StringConvertor<wchar_t>::ConvertFrom(src.c_str());
    }

    inline String FromWCString(const wchar_t* p_src)
    {
        return StringConvertor<Char>::ConvertFrom(p_src);
    }

    inline String FromWString(const std::wstring& src)
    {
        return StringConvertor<Char>::ConvertFrom(src.c_str());
    }

    inline std::wostream& operator << (std::wostream& out, const std::string& val)
    {
        return out << DecodeString(val.c_str());
    }

    inline std::wostream& operator << (std::wostream& out, const char* val)
    {
        return out << DecodeString(val);
    }

    template<typename Char, class T>
    std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& out, const std::optional<T>& val)
    {
        if (val)
        {
            using awl::operator <<;

            out << *val;
        }
        else
        {
            out << "empty";
        }

        return out;
    }
}

#ifdef AWL_QT

inline std::ostream& operator << (std::ostream& out, const QString& val)
{
    return out << val.toStdString();
}

inline std::wostream& operator << (std::wostream& out, const QString& val)
{
    return out << val.toStdWString();
}

inline std::istream& operator >> (std::istream& in, QString& val)
{
    std::string std_str;

    in >> std_str;

    val = QString::fromStdString(std_str);

    return in;
}

inline std::wistream& operator >> (std::wistream& in, QString& val)
{
    std::wstring std_str;

    in >> std_str;

    val = QString::fromStdWString(std_str);

    return in;
}

#endif //AWL_QT

#ifdef AWL_BOOST

namespace boost::multiprecision
{
    //boost::multiprecision does not have wostream operator
    template <class Backend, expression_template_option ExpressionTemplates>
    std::wostream& operator << (std::wostream& out, const number<Backend, ExpressionTemplates>& val)
    {
        std::ostringstream a_out;

        //we can't copy formatting from wstream
        //a_out.copyfmt(out);

        a_out << val;

        return out << awl::FromAString(a_out.str());
    }
}

#endif //AWL_BOOST

#ifdef AWL_INT_128

namespace awl
{
    //We need also implement std::setfill(static_cast<C>('0')) << std::setw(d.exponent())
    template <class C>
    std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, __uint128_t val)
    {
        //Calculate the number of decimal digits.
        std::streamsize len = 1;

        if (val != 0)
        {
            __uint128_t temp_val = val;

            while ((temp_val /= 10) != 0)
            {
                ++len;
            }
        }

        // There is always at least one digit.
        if (out.width() > 1 && len > out.width())
        {
            const std::streamsize diff = len - out.width();
            
            for (std::streamsize i = 0; i < diff; ++i)
            {
                out << out.fill();
            }
        }

        constexpr const C zero_symbol = static_cast<C>('0');

        std::size_t length = static_cast<std::size_t>(len);
        
        std::basic_string<C> result;
        result.reserve(length);
        
        if (val == 0)
        {
            result.push_back(zero_symbol);
        }
        else
        {
            do
            {
                const __uint128_t next_val = val / 10;
 
                const C digit_symbol = zero_symbol + static_cast<C>(val - next_val * 10);
                
                result.push_back(digit_symbol);
 
                val = next_val;
            }
            while (val != 0);
        }

        std::reverse(result.begin(), result.end());
 
        return (out << result);
    }

    template <class C>
    std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, __int128_t val)
    {
        if (out.width() != 0)
        {
            throw std::runtime_error("Width is not supported with negative integers.");
        }
        
        if (val < 0)
        {
            out << static_cast<C>('-');

            return out << static_cast<__uint128_t>(-val);
        }

        return out << static_cast<__uint128_t>(val);
    }
}

#endif //AWL_INT_128
