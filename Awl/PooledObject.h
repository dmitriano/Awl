#pragma once

#include "Awl/QuickList.h"

namespace awl
{
    AWL_DECLARE_QUICK_LINK(pooled_object_link)

    //A default constructible type derived from awl::quick_link.
    class pooled_object : public pooled_object_link
    {
    public:

        virtual void Finalize()
        {
        }

        virtual ~pooled_object() = default;
    };
}
