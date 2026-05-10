/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Separator.h"
#include "Awl/String.h"

#include <ranges>
namespace awl
{
    // An experimental class for writing a container elements into std::ostream.
    template <std::ranges::range R>
    class format_range
    {
    public:

        format_range(R& r) : _r(r) {}

        template<typename Char>
        friend std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& out, const format_range& fr)
        {
            out << "{";

            awl::basic_separator<Char> sep(',');

            for (const std::ranges::range_value_t<R>& val : fr._r)
            {
                out << sep << val;
            }

            out << "}";

            return out;
        }

    private:

        R& _r;
    };
}
