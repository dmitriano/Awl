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

        operator std::basic_string<C>() const { return out.str(); }

    private:
        
        std::basic_ostringstream<C> out;
    };

    using format = basic_format<Char>;
}
