/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Testing/AttributeProvider.h"

#define AWL_VARIABLE_ATTRIBUTE(attribute_type, attribute_name, default_val) attribute_type attribute_name(awl::testing::GetAttributeValue<attribute_type>( \
    context.ap, #attribute_name, default_val))

#define AWL_ATTRIBUTE(attribute_type, attribute_name, default_val) const AWL_VARIABLE_ATTRIBUTE(attribute_type, attribute_name, default_val)

#define AWL_FLAG(attribute_name) AWL_ATTRIBUTE(bool, attribute_name, false)
