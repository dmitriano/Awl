/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string_view>

namespace awl::io
{
    // Allow the user to rename structure fields by specializing FieldMap template class.
    template <class S>
    class FieldMap
    {
    public:

        static std::string_view GetNewName(std::string_view old_name)
        {
            return old_name;
        }
    };
}
