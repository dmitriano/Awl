/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Testing/AttributeProvider.h"

// Omit the default value to require an attribute from the provider.
#define AWL_VARIABLE_ATTRIBUTE(attribute_type, attribute_name, ...) attribute_type attribute_name(awl::testing::getAttributeValue<attribute_type>( \
    context.attributeProvider, #attribute_name __VA_OPT__(,) __VA_ARGS__))

#define AWL_ATTRIBUTE(attribute_type, attribute_name, ...) const AWL_VARIABLE_ATTRIBUTE(attribute_type, attribute_name __VA_OPT__(,) __VA_ARGS__)

#define AWL_VARIABLE_FLAG(attribute_name) AWL_VARIABLE_ATTRIBUTE(bool, attribute_name, false)

#define AWL_FLAG(attribute_name) const AWL_VARIABLE_FLAG(attribute_name)
