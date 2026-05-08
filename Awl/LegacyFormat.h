/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/LogString.h"

#include <source_location>
#include <type_traits>

namespace awl
{
    template <class C>
    class basic_format
    {
    public:
        basic_format(std::source_location location = std::source_location::current()) noexcept :
            m_location(location)
        {}
        
        template <typename T>
        basic_format & operator << (const T & val)
        {
            // All the users operators << should be declared
            // prior to the call site or in the global namespace (at least in GCC)
            // or in the namespace where T is defined.
            m_out << val;
            return *this;
        }

        std::basic_string<C> str() const { return m_out.str(); }

        operator std::basic_string<C>() const { return str(); }

#ifdef AWL_QT

        // Allow QT apps use this formatter.
        operator QString() const
        {
            if constexpr (std::is_same_v<C, char>)
            {
                return QString::fromStdString(str());
            }
            else
            {
                return QString::fromStdWString(str());
            }
        }

#endif

        operator LogString() const
        {
            return LogString(str(), m_location);
        }

        // std::endl flushes the output buffer but '\n' doesn't.
        static constexpr C endl = static_cast<C>('\n');

    private:
        
        std::basic_ostringstream<C> m_out;
        std::source_location m_location;
    };

    using format = basic_format<Char>;
    using aformat = basic_format<char>;
    using wformat = basic_format<wchar_t>;
}
