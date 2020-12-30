#pragma once

#include "Awl/String.h"

namespace awl
{
    template <class C>
    class basic_separator
    {
    public:

        basic_separator(C ch) : m_ch(ch)
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

        C content() const
        {
            return m_ch;
        }

    private:

        C m_ch;
        
        bool m_first = true;
    };

    template <class C>
    inline std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, basic_separator<C>& sep)
    {
        if (sep.first())
        {
            sep.hlop();
        }
        else
        {
            out << sep.content() << ' ';
        }
        
        return out;
    }

    using separator = basic_separator<Char>;
    using aseparator = basic_separator<char>;
    using wseparator = basic_separator<wchar_t>;
}
