#pragma once

#include  "Awl/Prototype.h"
#include  "Awl/Serializable.h"

#include <cassert>
#include <vector>
#include <unordered_set>

namespace awl::io
{
    using TypeNameVector = std::vector<std::string>;
    using PrototypeVector = std::vector<DetachedPrototype>;

    struct Metadata
    {
        TypeNameVector typeNames;
        PrototypeVector prototypes;

        AWL_SERIALIZABLE(typeNames, prototypes)
    };

    AWL_MEMBERWISE_EQUATABLE(Metadata)
}

//namespace awl::io
//{
//    using MetaTable = std::unordered_set<Metadata>;
//}
