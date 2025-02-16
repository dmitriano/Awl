/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Testing/AttributeProvider.h"

#define AWT_VARIABLE_ATTRIBUTE(attribute_type, attribute_name, default_val) attribute_type attribute_name(awl::testing::GetAttributeValue<attribute_type>( \
    context.ap, #attribute_name, default_val))

#define AWT_ATTRIBUTE(attribute_type, attribute_name, default_val) const AWT_VARIABLE_ATTRIBUTE(attribute_type, attribute_name, default_val)

#define AWT_FLAG(attribute_name) AWT_ATTRIBUTE(bool, attribute_name, false)
