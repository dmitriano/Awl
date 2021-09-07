#pragma once

#include "Awl/TransformIterator.h"
#include "Awl/Range.h"

namespace awl
{
    template <class Container, class UnaryFunction>
    auto make_transform_range(Container & container, UnaryFunction func)
    {
        return make_range(make_transform_iterator(container.begin(), func), make_transform_iterator(container.end(), func));
    }

    template <class Container, class UnaryFunction>
    auto make_transform_crange(const Container & container, UnaryFunction func)
    {
        return make_range(make_transform_iterator(container.begin(), func), make_transform_iterator(container.end(), func));
    }

    template <class Container, class UnaryFunction>
    auto make_transform_range(const Container & container, UnaryFunction func)
    {
        return make_transform_crange(container, func);
    }
}
