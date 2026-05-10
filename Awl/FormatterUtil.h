/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

#include <sstream>

namespace awl
{
    template <class CharT, class T>
    std::basic_string<CharT> to_basic_string_via_ostream(const T& val)
    {
        std::ostringstream out;

        out << val;

        return StringConvertor<CharT>::convertFrom(out.str().c_str());
    }
}
