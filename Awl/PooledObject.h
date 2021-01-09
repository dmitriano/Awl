#pragma once

#include "Awl/QuickList.h"

namespace awl
{
    //A default constructible type derived from awl::quick_link.
    class pooled_object : public awl::quick_link
    {
    public:

        virtual void Finalize()
        {
        }

        virtual ~pooled_object() = default;
    };
}
