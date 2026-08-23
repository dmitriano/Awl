/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

namespace awl
{
    template <class C>
    class basic_separator
    {
    public:

        basic_separator(C ch) : _sep(1, ch)
        {
            _sep += ' ';
        }

        basic_separator(std::basic_string<C> s) : _sep(s)
        {}

        bool first() const
        {
            return _first;
        }

        void hlop()
        {
            _first = false;
        }

        const std::basic_string<C>& content()
        {
            return _sep;
        }

    private:

        std::basic_string<C> _sep;
        
        bool _first = true;
    };

    template <class C>
    std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, basic_separator<C>& sep)
    {
        if (sep.first())
        {
            sep.hlop();
        }
        else
        {
            out << sep.content();
        }
        
        return out;
    }

    using separator = basic_separator<Char>;
    using aseparator = basic_separator<char>;
    using wseparator = basic_separator<wchar_t>;
}
