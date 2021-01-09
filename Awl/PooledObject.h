#pragma once

#include "Awl/QuickList.h"

namespace awl
{
    //A default constructible type derived from awl::quick_link.
    class pooled_object : public basic_quick_link<pooled_object>
    {
    private:

        using Base = basic_quick_link<pooled_object>;

    public:

        using Base::Base;

        virtual void Finalize()
        {
        }

        virtual ~pooled_object() = default;
    };
}
