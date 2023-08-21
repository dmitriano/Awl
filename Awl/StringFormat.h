/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

// Can't figure out why do we need this header with GCC but not with MSVC.
#if defined(__GNUC__)
#include "Awl/Time.h"
#endif

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
