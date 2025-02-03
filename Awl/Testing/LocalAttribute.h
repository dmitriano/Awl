/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Testing/AttributeProvider.h"

namespace awl::testing
{
    template <class T, class Provider>
    T GetAttributeValue(Provider& provider, const Char* type, const Char* name, T default_val)
    {
        static_cast<void>(type);

        T val;

        if (!provider.TryGet(name, val))
        {
            return default_val;
        }

        return val;
    }
}

#define AWT_ATTRIBUTE(attribute_type, attribute_name, default_val) const attribute_type attribute_name(awl::testing::GetAttributeValue<attribute_type>( \
    context.ap, _T(#attribute_type), _T(#attribute_name), default_val))

#define AWT_FLAG(attribute_name) AWT_ATTRIBUTE(bool, attribute_name, false)
