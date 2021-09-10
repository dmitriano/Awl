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

        basic_separator(C ch) : m_sep(1, ch)
        {
            m_sep += ' ';
        }

        basic_separator(std::basic_string<C> s) : m_sep(s)
        {
        }

        bool first() const
        {
            return m_first;
        }

        void hlop()
        {
            m_first = false;
        }

        const std::basic_string<C>& content()
        {
            return m_sep;
        }

    private:

        std::basic_string<C> m_sep;
        
        bool m_first = true;
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
