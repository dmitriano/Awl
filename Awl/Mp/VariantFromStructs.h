/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Mp/TupleToVariant.h"
#include "Awl/Mp/FlattenTuple.h"

#include "Awl/Tuplizable.h"

namespace awl::mp
{
    template <class T>
    using recursive_tuple = decltype(flatten_struct<T>());

    template <class T>
    using variant_from_struct = tuple_to_variant<recursive_tuple<T>>;

    template <class... Ts>
    auto variant_from_structs_func()
    {
        std::tuple<Ts...> t;
        return flatten_tuple(t);
    }

    template <class... Ts>
    using variant_from_structs = tuple_to_variant<decltype(variant_from_structs_func<Ts...>())>;
}
