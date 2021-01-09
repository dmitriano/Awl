#pragma once

#include "Awl/QuickList.h"

namespace awl
{
    //A default constructible type derived from awl::quick_link.
    class pooled_object : public awl::quick_link
    {
    public:

        using quick_link::quick_link;

        virtual void Finalize()
        {
        }

        virtual ~pooled_object() = default;
    };
}
