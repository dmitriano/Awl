/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Testing/AttributeProvider.h"

namespace awl::testing
{
    template <class T, class Provider>
    T GetAttributeValue(Provider& provider, const char* name, T default_val)
    {
        ProviderAdapter<T, Provider> adapter{ provider };

        T val;

        if (!adapter.TryGet(name, val))
        {
            return default_val;
        }

        return val;
    }
}

#define AWT_ATTRIBUTE(attribute_type, attribute_name, default_val) const attribute_type attribute_name(awl::testing::GetAttributeValue<attribute_type>( \
    context.ap, #attribute_name, default_val))

#define AWT_FLAG(attribute_name) AWT_ATTRIBUTE(bool, attribute_name, false)
