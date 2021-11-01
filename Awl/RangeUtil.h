/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ranges>

namespace awl
{
    template <class R, class Value>
    concept range_over = std::ranges::range<R> &&
        std::same_as<std::ranges::range_value_t<R>, Value>;

    template <std::ranges::range R, class Compare>
    bool position_changed(const R& r, const Compare& comp, const std::ranges::iterator_t<R>& old_iter)
    {
        bool pos_changed = false;

        if (old_iter != r.begin())
        {
            auto i_prev = old_iter;
            --i_prev;

            if (!comp(*i_prev, *old_iter))
            {
                pos_changed = true;
            }
        }

        if (!pos_changed)
        {
            auto i_next = old_iter;
            ++i_next;

            if (i_next != r.end())
            {
                if (!comp(*old_iter, *i_next))
                {
                    pos_changed = true;
                }
            }
        }

        return pos_changed;
    }
}
