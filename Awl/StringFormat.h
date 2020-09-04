#pragma once

#include "Awl/String.h"

namespace awl
{
    template <class C>
    class basic_format
    {
    public:
        
        template <typename T>
        basic_format & operator << (const T & val)
        {
            out << val;
            return *this;
        }

        std::basic_string<C> str() const { return out.str(); }

        operator std::basic_string<C>() const { return str(); }

    private:
        
        std::basic_ostringstream<C> out;
    };

    using format = basic_format<Char>;
    using aformat = basic_format<char>;
    using wformat = basic_format<wchar_t>;
}
