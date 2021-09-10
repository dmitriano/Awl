/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>

namespace awl
{
    template<class R, class...Args>
    auto y_combinator = [](auto f)
    {
        auto action = [=](auto action) -> std::function<R(Args...)>
        {
            return [=](Args&&... args)->R
            {
                return f(action(action), std::forward<Args>(args)...);
            };
        };
        
        return action(action);
    };
}
