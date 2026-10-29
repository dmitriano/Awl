#pragma once

#include "Awl/BitMap.h"

namespace awl
{
    // Reading options are propagated to nested serializers. No options are enabled by default.
    AWL_SEQUENTIAL_ENUM(JsonSerializationOptions, RejectUnknownFields)
}

AWL_ENUM_TRAITS(awl, JsonSerializationOptions)
