#pragma once

#include "Awl/String.h"

namespace awl
{
    class format
    {
    public:
        
        template <typename T>
        format & operator << (const T & val)
        {
            out << val;
            return *this;
        }

        operator String() const { return out.str(); }

    private:
        
        ostringstream out;
    };
}
