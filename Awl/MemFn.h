/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <type_traits>
#include <utility>

namespace awl
{
    // For using with std::ranges::filter without std::bind.
    template <class Field, class Proj>
    class projected_equal_to
    {
    public:

        projected_equal_to(Field field, Proj proj) :
            _proj(proj),
            _field(std::move(field))
        {}

        // object_val can be of type std::shared_ptr, for example.
        bool operator() (const auto& object_val) const
        {
            return std::invoke(_proj, object_val) == _field;
        }

    private:

        Proj _proj;
        Field _field;
    };

    template <class Value, class Field>
    auto mem_fn_equal_to(Field(Value::* field_ptr)() const, std::decay_t<Field> field)
    {
        return projected_equal_to<Field, decltype(field_ptr)>(field, field_ptr);
    }

    template <class Value, class Field>
    auto mem_fn_equal_to(Field Value::*field_ptr, Field field)
    {
        return projected_equal_to<Field, decltype(field_ptr)>(field, field_ptr);
    }
}
