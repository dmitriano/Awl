/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Mp/TupleToVariant.h"
#include "Awl/Mp/TypeCollector.h"

namespace awl::mp
{
    template <class T>
    using variant_from_struct = tuple_to_variant<typename type_collector<T>::Tuple>;

    template <class... Ts>
    using variant_from_structs = tuple_to_variant<tuple_cat_t<typename type_collector<Ts>::Tuple ...>>;
}
